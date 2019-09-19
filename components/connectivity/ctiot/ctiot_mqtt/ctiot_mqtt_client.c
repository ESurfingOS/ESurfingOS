#include "ctiot_mqtt_client.h"
#include "los_base.h"
#include "log/atiny_log.h"
#include "MQTTClient.h"
#include "cJSON.h"

#define MQTT_VERSION_3_1 (3)
#define MQTT_VERSION_3_1_1 (4)

#define VARIABLE_SIZE (4 + 1)

#define MQTT_TIME_BUF_LEN 11

#define STRING_MAX_LEN 256

#define IS_VALID_NAME_LEN(name) (strlen((name)) <= STRING_MAX_LEN)

#define CTIOT_MEM_FREE(mem) \
do\
{\
    if(NULL != (mem))\
    {\
       atiny_free(mem);\
       (mem) = NULL;\
    }\
}while(0)

typedef enum{
    CTIOT_INT,
    CTIOT_DOUBLE,
    CTIOT_FLOAT,
    CTIOT_STR,
    CTIOT_ENUM,
    CTIOT_BOOL,
    CTIOT_DATE,
}CTIOT_PARA_TYPE;
typedef struct
{
    int max;
    int min;
    int val;
}CTIOT_INT_ITEM;

typedef struct
{
    double max;
    double min;
    double val;
}CTIOT_DOUBLE_ITEM;

typedef struct
{
    float max;
    float min;
    float val;
}CTIOT_FLOAT_ITEM;

typedef struct
{
    int maxLen;
    int minLen;
    char *val;
}CTIOT_STR_ITEM;

typedef struct
{
    int maxLen;
    int minLen;
    int val;
}CTIOT_ENUM_ITEM;

typedef struct
{
    int max;
    int min;
    int val;
}CTIOT_BOOL_ITEM;

typedef struct
{
    int max;
    int min;
    long long val;
}CTIOT_DATE_ITEM;

typedef struct
{
    CTIOT_PARA_TYPE ctiotParaType;
    char *paraName;
	union{
        CTIOT_STR_ITEM ctiotStr;
        CTIOT_FLOAT_ITEM ctiotFloat;
        CTIOT_DOUBLE_ITEM ctiotDouble;
        CTIOT_DATE_ITEM ctiotDate;
        CTIOT_INT_ITEM ctiotInt;
        CTIOT_BOOL_ITEM ctiotBool;
        CTIOT_ENUM_ITEM ctiotEnum;
    }u;
}CTIOT_PARAM_ITEM;

#define MAX_PARA_COUNT 10

typedef struct ctiot_mqtt_para{
    int count;
    CTIOT_PARAM_ITEM paraList[MAX_PARA_COUNT];
}CTIOT_MQTT_PARA;

typedef void (* CTIOT_CB)(MessageData *md);

struct mqtt_client_tag_s
{
	mqtt_device_info_s device_info;
	MQTTClient client;
	mqtt_param_s params;
	void *ctiotCbFunc;
	char *sub_topic;
	uint8_t init_flag;
	uint8_t reserve[3];
};

static uint8_t g_mqtt_sendbuf[MQTT_SENDBUF_SIZE];

/* reserve 1 byte for string end 0 for jason */
static uint8_t g_mqtt_readbuf[MQTT_READBUF_SIZE + 1];

mqtt_client_s g_mqtt_client;


static int ctiot_mqtt_publish_msg(mqtt_client_s *phandle, const char *msg,  uint32_t msg_len, mqtt_qos_e qos, char* topic);



static void ctiot_mqtt_register_cbs(void *cbs)
{
	CTIOT_CB_FUNC *p = cbs;
	
	if(NULL == cbs)
		return;
	
    g_mqtt_client.ctiotCbFunc = p;
}


static void ctiot_mqtt_free_params(mqtt_param_s *param)
{

    CTIOT_MEM_FREE(param->server_ip);
    CTIOT_MEM_FREE(param->server_port);
}

static int ctiot_mqtt_check_param(const mqtt_param_s *param)
{
    if ((param->server_ip == NULL)
        || (param->server_port == NULL)
        || (param->info.security_type >= MQTT_SECURITY_TYPE_MAX))
    {
        ATINY_LOG(LOG_FATAL, "invalid param, sec type %d", param->info.security_type);
        return ATINY_ARG_INVALID;
    }

    return ATINY_OK;
}

static int ctiot_mqtt_dup_param(mqtt_param_s *dest, const mqtt_param_s *src)
{
    memset(dest, 0, sizeof(*dest));

    dest->info.security_type = src->info.security_type;

    dest->server_ip = atiny_strdup(src->server_ip);
    if(NULL == dest->server_ip)
    {
        ATINY_LOG(LOG_FATAL, "atiny_strdup NULL");
        return ATINY_MALLOC_FAILED;
    }

    dest->server_port = atiny_strdup(src->server_port);
    if(NULL == dest->server_port)
    {
        ATINY_LOG(LOG_FATAL, "atiny_strdup NULL");
        goto mqtt_param_dup_failed;
    }

    return ATINY_OK;

mqtt_param_dup_failed:
    ctiot_mqtt_free_params(dest);
    return ATINY_MALLOC_FAILED;
}

static void ctiot_mqtt_free_device_info(mqtt_device_info_s *info)
{

    CTIOT_MEM_FREE(info->password);
    if(MQTT_STATIC_CONNECT == info->connection_type)
    {
        CTIOT_MEM_FREE(info->u.s_info.deviceid);
    }
}

static int ctiot_mqtt_check_device_info(const mqtt_device_info_s *info)
{
    if((info->connection_type >= MQTT_MAX_CONNECTION_TYPE)
        || (info->codec_mode >= MQTT_MAX_CODEC_MODE)
        || (NULL == info->password)
        || (!IS_VALID_NAME_LEN(info->password)))
    {
        ATINY_LOG(LOG_FATAL, "invalid device info con_type %d codec_mode %d ",
            info->connection_type, info->codec_mode);
        return ATINY_ARG_INVALID;
    }

    if ((info->connection_type == MQTT_STATIC_CONNECT)
        && ((NULL == info->u.s_info.deviceid)
        || (!IS_VALID_NAME_LEN(info->u.s_info.deviceid))))
    {
        ATINY_LOG(LOG_FATAL, "invalid static device info con_type %d codec_mode %d",
            info->connection_type, info->codec_mode);
        return ATINY_ARG_INVALID;
    }
    else if(info->connection_type != MQTT_STATIC_CONNECT)
    {
        ATINY_LOG(LOG_FATAL, "invalid connection type, con_type %d ",
            info->connection_type);
    }

    return ATINY_OK;

}

static int ctiot_mqtt_dup_device_info(mqtt_device_info_s *dest, const mqtt_device_info_s *src)
{
    memset(dest, 0, sizeof(*dest));
    dest->connection_type = src->connection_type;
    dest->codec_mode = src->codec_mode;
    dest->password = atiny_strdup(src->password);
    if (NULL == dest->password)
    {
        ATINY_LOG(LOG_FATAL, "atiny_strdup fail");
        return ATINY_MALLOC_FAILED;
    }

    if(MQTT_STATIC_CONNECT == src->connection_type)
    {
        dest->u.s_info.deviceid = atiny_strdup(src->u.s_info.deviceid);
        if (NULL == dest->u.s_info.deviceid)
        {
            ATINY_LOG(LOG_FATAL, "atiny_strdup fail");
            goto MALLOC_FAIL;
        }
    }

    return ATINY_OK;

MALLOC_FAIL:
    ctiot_mqtt_free_device_info(dest);
    return ATINY_MALLOC_FAILED;
}

static bool ctiot_mqtt_is_connectting_with_deviceid(const mqtt_client_s* handle)
{
    return (MQTT_STATIC_CONNECT == handle->device_info.connection_type);
}

static void ctiot_mqtt_destroy_data_connection_info(MQTTPacket_connectData *data)
{
    CTIOT_MEM_FREE(data->clientID.cstring);
    CTIOT_MEM_FREE(data->password.cstring);
}



static int ctiot_mqtt_get_connection_info(mqtt_client_s* handle, MQTTPacket_connectData *data)
{
    char time[MQTT_TIME_BUF_LEN];
    char *password;

    if (ctiot_mqtt_is_connectting_with_deviceid(handle))
    {
        if (handle->device_info.connection_type == MQTT_STATIC_CONNECT)
        {
            password = handle->device_info.password;
        }
        ATINY_LOG(LOG_INFO, "try static connect");
    }

    data->clientID.cstring = atiny_strdup(handle->device_info.u.s_info.deviceid);
    if (data->clientID.cstring == NULL)
    {
        return ATINY_MALLOC_FAILED;
    }

    data->username.cstring = handle->device_info.u.s_info.deviceid;
    data->password.cstring = password;

    if (data->password.cstring == NULL)
    {
        return ATINY_ERR;
    }

    ATINY_LOG(LOG_FATAL, "send user %s client %s", data->username.cstring,
                data->clientID.cstring);

    return ATINY_OK;
}

static int ctiot_mqtt_modify_payload(void *md)
{
    char *end = ((char *)((MessageData *)md)->message->payload) + ((MessageData *)md)->message->payloadlen;
    static uint32_t callback_err;

    if ((end >= (char *)g_mqtt_readbuf) && (end < (char *)(g_mqtt_readbuf + sizeof(g_mqtt_readbuf))))
    {
         *end = '\0';
         return ATINY_OK;
    }

    ATINY_LOG(LOG_ERR, "not expect msg callback err, pl %p, len %ld, err num %ld", ((MessageData *)md)->message->payload, ((MessageData *)md)->message->payloadlen, ++callback_err);

    return ATINY_ERR;
}

static void ctiot_mqtt_disconnect( MQTTClient *client, Network *n)
{
    if (MQTTIsConnected(client))
    {
        (void)MQTTDisconnect(client);
    }
    NetworkDisconnect(n);

    ATINY_LOG(LOG_ERR, "mqtt_disconnect");
}

static inline void ctiot_mqtt_inc_fail_cnt(int32_t *conn_failed_cnt)
{
    if(*conn_failed_cnt < MQTT_CONN_FAILED_MAX_TIMES)
    {
        (*conn_failed_cnt)++;
    }
}

static void ctiot_mqtt_proc_connect_err( MQTTClient *client, Network *n, int32_t *conn_failed_cnt)
{
    ctiot_mqtt_inc_fail_cnt(conn_failed_cnt);
    ctiot_mqtt_disconnect(client, n);
}

static mqtt_security_info_s *mqtt_get_security_info(void)
{
    mqtt_client_s* handle = &g_mqtt_client;
    return &handle->params.info;
}


int ctiot_mqtt_init(const mqtt_param_s *params,void *callback_struct, mqtt_client_s **phandle)
{
    cJSON_InitHooks(NULL);
    if (params == NULL || phandle == NULL
        || ctiot_mqtt_check_param(params) != ATINY_OK)
    {
        ATINY_LOG(LOG_FATAL, "Invalid args");
        return ATINY_ARG_INVALID;
    }

    if (g_mqtt_client.init_flag)
    {
        ATINY_LOG(LOG_FATAL, "mqtt reinit");
        return ATINY_ERR;
    }

    memset(&g_mqtt_client, 0, sizeof(g_mqtt_client));

    if (ATINY_OK != ctiot_mqtt_dup_param(&(g_mqtt_client.params), params))
    {
        return ATINY_MALLOC_FAILED;
    }

    *phandle = &g_mqtt_client;

    ctiot_mqtt_register_cbs(callback_struct);

    g_mqtt_client.init_flag = true;

    return ATINY_OK;
}

int ctiot_mqtt_login(const mqtt_device_info_s* device_info, mqtt_client_s* handle)
{
    Network n;
    MQTTClient *client = NULL;
    mqtt_param_s *params;
    int rc;
    int32_t conn_failed_cnt = 0;
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    Timer timer;
    int result = ATINY_ERR;

    if (NULL == handle)
    {
        ATINY_LOG(LOG_FATAL, "handle null");
        return ATINY_ARG_INVALID;
    }

    if((device_info == NULL)
        || (ctiot_mqtt_check_device_info(device_info) != ATINY_OK))
    {
        ATINY_LOG(LOG_FATAL, "parameter invalid");
        result = ATINY_ARG_INVALID;
        goto  ctiot_login_quit;
    }

    client = &(handle->client);
    params = &(handle->params);

    rc = ctiot_mqtt_dup_device_info(&(handle->device_info), device_info);
    if (rc != ATINY_OK)
    {
        goto  ctiot_login_quit;
    }

    NetworkInit(&n, mqtt_get_security_info);

    memset(client, 0x0, sizeof(MQTTClient));
    rc = MQTTClientInit(client, &n, MQTT_COMMAND_TIMEOUT_MS, g_mqtt_sendbuf, MQTT_SENDBUF_SIZE, g_mqtt_readbuf, MQTT_READBUF_SIZE);
    if (rc != MQTT_SUCCESS)
    {
        ATINY_LOG(LOG_FATAL, "MQTTClientInit fail,rc %d", rc);
        goto  ctiot_login_quit;
    }

    data.willFlag = 0;
    data.MQTTVersion = MQTT_VERSION_3_1_1;
    data.keepAliveInterval = MQTT_KEEPALIVE_INTERVAL_S;
    data.cleansession = true;

    while(true)
    {
        if(conn_failed_cnt > 0)
        {
            ATINY_LOG(LOG_INFO, "reconnect delay : %d", conn_failed_cnt);
            (void)LOS_TaskDelay(MQTT_CONN_FAILED_BASE_DELAY << conn_failed_cnt);
        }

        rc = NetworkConnect(&n, params->server_ip, atoi(params->server_port));
        if(rc != 0)
        {
            ATINY_LOG(LOG_ERR, "NetworkConnect fail: %d", rc);
            ctiot_mqtt_inc_fail_cnt(&conn_failed_cnt);
            continue;
        }

        if(ctiot_mqtt_get_connection_info(handle, &data) != ATINY_OK)
        {
            ctiot_mqtt_destroy_data_connection_info(&data);
            ctiot_mqtt_proc_connect_err(client, &n, &conn_failed_cnt);
            continue;
        }

        rc = MQTTConnect(client, &data);
        ctiot_mqtt_destroy_data_connection_info(&data);
        ATINY_LOG(LOG_DEBUG, "CONNACK : %d", rc);
        if(MQTT_SUCCESS != rc)
        {
            ATINY_LOG(LOG_ERR, "MQTTConnect failed %d", rc);
            ctiot_mqtt_proc_connect_err(client, &n, &conn_failed_cnt);
            continue;
        }

		if(ATINY_OK != ctiot_mqtt_subscribe(handle))       
        {
            ATINY_LOG(LOG_ERR, "mqtt_subscribe_topic failed");
            ctiot_mqtt_proc_connect_err(client, &n, &conn_failed_cnt);
            continue;
        }

        conn_failed_cnt = 0;
        if (!ctiot_mqtt_is_connectting_with_deviceid(handle))
        {
            TimerInit(&timer);
            TimerCountdownMS(&timer, MQTT_WRITE_FOR_SECRET_TIMEOUT);
        }
        while (rc >= 0 && MQTTIsConnected(client))
        {
            rc = MQTTYield(client, MQTT_EVENTS_HANDLE_PERIOD_MS);
        }

        ctiot_mqtt_disconnect(client, &n);
    }

    result = ATINY_OK;
ctiot_login_quit:
    ctiot_mqtt_free_params(&(handle->params));
    (void)atiny_task_mutex_lock(&client->mutex);
    ctiot_mqtt_free_device_info(&(handle->device_info));
    (void)atiny_task_mutex_unlock(&client->mutex);
    MQTTClientDeInit(client);
    handle->init_flag = false;
    return result;
}



int ctiot_mqtt_isconnected(mqtt_client_s* phandle)
{
    if (NULL == phandle)
    {
        ATINY_LOG(LOG_ERR, "invalid args");
        return false;
    }
    return ctiot_mqtt_is_connectting_with_deviceid(phandle) && MQTTIsConnected(&(phandle->client));
}

static CTIOT_MSG_STATUS ctiot_mqtt_validate(CTIOT_MQTT_PARA para)
{
    CTIOT_MSG_STATUS ret = CTIOT_SUCCESS;
    int i = 0;
    for(;i < para.count;i++){
        switch(para.paraList[i].ctiotParaType){
            case CTIOT_INT:
			{
                if(para.paraList[i].u.ctiotInt.val < para.paraList[i].u.ctiotInt.min || para.paraList[i].u.ctiotInt.val > para.paraList[i].u.ctiotInt.max){
                    return CTIOT_PARA_ERROR;
                }
                break;
			}
            case CTIOT_DOUBLE:
			{
                if(para.paraList[i].u.ctiotDouble.val < para.paraList[i].u.ctiotDouble.min || para.paraList[i].u.ctiotDouble.val > para.paraList[i].u.ctiotDouble.max){
                    return CTIOT_PARA_ERROR;
                }
                break;
			}
            case CTIOT_FLOAT:
			{
                if(para.paraList[i].u.ctiotFloat.val < para.paraList[i].u.ctiotFloat.min || para.paraList[i].u.ctiotFloat.val > para.paraList[i].u.ctiotFloat.max){
                    return CTIOT_PARA_ERROR;
                }
                break;
			}
            case CTIOT_STR:
			{
                if(strlen(para.paraList[i].u.ctiotStr.val) < para.paraList[i].u.ctiotStr.minLen || strlen(para.paraList[i].u.ctiotStr.val) > para.paraList[i].u.ctiotStr.maxLen){
                    return CTIOT_PARA_ERROR;
                }
                break;
			}
            case CTIOT_ENUM:
			{
                break;
			}
            case CTIOT_BOOL:
			{
                break;
			}
            case CTIOT_DATE:
			{
                break;
			}
            default:
				return CTIOT_TYPE_ERROR;
        }
    }
    return ret;
}

static CTIOT_MSG_STATUS ctiot_mqtt_msg_encode(CTIOT_MQTT_PARA para,char** payload)
{
    CTIOT_MSG_STATUS ret = CTIOT_SUCCESS;
    cJSON *root = cJSON_CreateObject();
    int i = 0;
    for(;i < para.count;i++){
        switch(para.paraList[i].ctiotParaType){
            case CTIOT_INT:
			{
                cJSON_AddItemToObject(root,para.paraList[i].paraName,cJSON_CreateNumber(para.paraList[i].u.ctiotInt.val));
                break;
			}
            case CTIOT_DOUBLE:
			{
                cJSON_AddItemToObject(root,para.paraList[i].paraName,cJSON_CreateNumber(para.paraList[i].u.ctiotDouble.val));
                break;
			}
            case CTIOT_FLOAT:
			{
                cJSON_AddItemToObject(root,para.paraList[i].paraName,cJSON_CreateNumber(para.paraList[i].u.ctiotFloat.val));
                break;
			}
            case CTIOT_STR:
			{
                cJSON_AddItemToObject(root,para.paraList[i].paraName,cJSON_CreateString(para.paraList[i].u.ctiotStr.val));
                break;
			}
            case CTIOT_ENUM:
            {
                cJSON_AddItemToObject(root,para.paraList[i].paraName,cJSON_CreateNumber(para.paraList[i].u.ctiotEnum.val));
                break;
            }
            case CTIOT_BOOL:
			{
                cJSON_AddItemToObject(root,para.paraList[i].paraName,cJSON_CreateBool(para.paraList[i].u.ctiotBool.val));
                break;
			}
            case CTIOT_DATE:
			{
                cJSON_AddItemToObject(root,para.paraList[i].paraName,cJSON_CreateNumber(para.paraList[i].u.ctiotDate.val));
                break;
			}
            default:
			{
                if(root){
                    cJSON_Delete(root);
                }
                return CTIOT_TYPE_ERROR;
			}
        }
    }
    (*payload) = cJSON_Print(root);
	ATINY_LOG(LOG_DEBUG, "report:%s!",(*payload));
		
    cJSON_Delete(root);

    return ret;
}

static CTIOT_MSG_STATUS ctiot_mqtt_msg_response_encode(CTIOT_MQTT_PARA para,int taskId,char** payload)
{
    CTIOT_MSG_STATUS ret = CTIOT_SUCCESS;
    cJSON *root = cJSON_CreateObject();
    cJSON_AddItemToObject(root,"taskId",cJSON_CreateNumber(taskId));

    cJSON *paylaodItem = cJSON_CreateObject();
    int i = 0;
    for(;i < para.count;i++){
        switch(para.paraList[i].ctiotParaType){
            case CTIOT_INT:
			{
                cJSON_AddItemToObject(paylaodItem,para.paraList[i].paraName,cJSON_CreateNumber(para.paraList[i].u.ctiotInt.val));
                break;
			}
            case CTIOT_DOUBLE:
			{
                cJSON_AddItemToObject(paylaodItem,para.paraList[i].paraName,cJSON_CreateNumber(para.paraList[i].u.ctiotDouble.val));
                break;
			}
            case CTIOT_FLOAT:
			{
                cJSON_AddItemToObject(paylaodItem,para.paraList[i].paraName,cJSON_CreateNumber(para.paraList[i].u.ctiotFloat.val));
                break;
			}
            case CTIOT_STR:
			{
                cJSON_AddItemToObject(paylaodItem,para.paraList[i].paraName,cJSON_CreateString(para.paraList[i].u.ctiotStr.val));
                break;
			}
            case CTIOT_ENUM:
            {
                cJSON_AddItemToObject(paylaodItem,para.paraList[i].paraName,cJSON_CreateNumber(para.paraList[i].u.ctiotEnum.val));
                break;
            }
            case CTIOT_BOOL:
			{
                cJSON_AddItemToObject(paylaodItem,para.paraList[i].paraName,cJSON_CreateBool(para.paraList[i].u.ctiotBool.val));
                break;
			}
            case CTIOT_DATE:
			{
                cJSON_AddItemToObject(paylaodItem,para.paraList[i].paraName,cJSON_CreateNumber(para.paraList[i].u.ctiotDate.val));
                break;
			}
            default:
            {
                if(root){
                    cJSON_Delete(root);
                }
                if(paylaodItem){
                    cJSON_Delete(paylaodItem);
                }
                return CTIOT_TYPE_ERROR;
            }
        }
    }
    cJSON_AddItemToObject(root,"resultPayload",paylaodItem);
    (*payload) = cJSON_Print(root);
	ATINY_LOG(LOG_DEBUG, "response:%s!",(*payload));
    cJSON_Delete(root);
    return ret;
}

/*
#define cJSON_False  (1 << 0)
#define cJSON_True   (1 << 1)
#define cJSON_NULL   (1 << 2)
#define cJSON_Number (1 << 3)
#define cJSON_String (1 << 4)
#define cJSON_Array  (1 << 5)
*/
static CTIOT_MQTT_PARA ctiot_mqtt_json_parsing(char *json)
{
    CTIOT_MQTT_PARA para = {0};
    int count = 0;
    cJSON *root = cJSON_Parse(json);
    cJSON *payloadItem = cJSON_GetObjectItem(root,"payload");
    cJSON *taskIdItem = cJSON_GetObjectItem(root,"taskId");

    para.paraList[count].ctiotParaType = CTIOT_INT;
    para.paraList[count].u.ctiotInt.val = taskIdItem->valueint;
	para.paraList[count].paraName = "taskId";
    count ++;

    cJSON *c = payloadItem->child;
    while(c != NULL)
    {
        switch(c->type){
            case cJSON_False:
            case cJSON_True:
                para.paraList[count].ctiotParaType = CTIOT_BOOL;
                para.paraList[count].u.ctiotBool.val = c->valueint;
                para.paraList[count].paraName = c->string;
                break;
            case cJSON_Number:
                para.paraList[count].ctiotParaType = CTIOT_DOUBLE;
                para.paraList[count].u.ctiotDouble.val = c->valuedouble;
                para.paraList[count].paraName = c->string;
                break;
            case cJSON_String:
                para.paraList[count].ctiotParaType = CTIOT_STR;
                para.paraList[count].u.ctiotStr.val = c->valuestring;
                para.paraList[count].paraName = c->string;
                break;
            case cJSON_NULL:
                break;
        }
        count ++;
        //ATINY_LOG(LOG_DEBUG, "response: %s!",c->string);
        c = c->next;
        //ATINY_LOG(LOG_DEBUG, "response: %d!",count);
    }
    para.count = count;
    return para;
}

CTIOT_MSG_STATUS ctiot_mqtt_msg_publish(char *topic,mqtt_qos_e qos,char* payload)
{
	CTIOT_MSG_STATUS ret = CTIOT_SUCCESS;
	ATINY_LOG(LOG_DEBUG, "ctiot_mqtt_msg_publish:%s  !", payload);

	int rc = ctiot_mqtt_publish_msg(&g_mqtt_client, payload, strlen(payload), qos, topic);
	atiny_free(payload);
	if (rc != 0)
	{
		ret = CTIOT_PUBLISH_ERROR;
	}
	return ret;
}
static int ctiot_mqtt_publish_msg(mqtt_client_s *phandle, const char *msg,  uint32_t msg_len, mqtt_qos_e qos, char* topic)
{
    MQTTMessage message;
    int rc;

    if ((phandle == NULL) || (msg == NULL) || (msg_len <= 0)
        || (qos >= MQTT_QOS_MAX))
    {
        ATINY_LOG(LOG_FATAL, "Parameter invalid");
        return ATINY_ARG_INVALID;
    }

    if (!ctiot_mqtt_isconnected(phandle))
    {
        ATINY_LOG(LOG_FATAL, "not connected");
        return ATINY_ERR;
    }

    if (topic == NULL)
    {
        return ATINY_MALLOC_FAILED;
    }
    memset(&message, 0, sizeof(message));
    message.qos = (enum QoS)qos;
    message.payload = (void *)msg;
    message.payloadlen = strlen(msg);
    rc = MQTTPublish(&phandle->client, topic, &message);
    if (rc != MQTT_SUCCESS)
    {
        ATINY_LOG(LOG_FATAL, "MQTTPublish fail,rc %d", rc);
        return ATINY_ERR;
    }
    else
    {
        ATINY_LOG(LOG_FATAL, "MQTTPublish payload %s", msg);
    }
    return ATINY_OK;
}


static void ctiot_cmd_dn_entry(MessageData *md)
{
	if ((md == NULL) || (md->message == NULL) || (ctiot_mqtt_modify_payload(md) != ATINY_OK))
	{
		ATINY_LOG(LOG_FATAL, "null point");
		return;
	}

	char *payload = md->message->payload;
	CTIOT_CB_FUNC *ctiot_callback = g_mqtt_client.ctiotCbFunc;

	if (ctiot_callback != NULL && ctiot_callback->ctiot_mqtt_cmd_dn_tr != NULL)
	{
		ATINY_LOG(LOG_INFO, "receive msg: %s", payload);
		ctiot_callback->ctiot_mqtt_cmd_dn_tr(payload);
	}
}

int ctiot_mqtt_subscribe(void* mhandle)
{
	struct mqtt_client_tag_s *handle = mhandle;
	char *topic;
	CTIOT_CB topic_callback = NULL;
	int rc;

	if (handle->sub_topic)
	{
		(void)MQTTSetMessageHandler(&handle->client, handle->sub_topic, NULL);
		atiny_free(handle->sub_topic);
		handle->sub_topic = NULL;
	}
	topic = "device_control";
	topic_callback = ctiot_cmd_dn_entry;
	(void)MQTTSetMessageHandler(&handle->client, topic, topic_callback);
	ATINY_LOG(LOG_INFO, "subcribe static topic: %s", topic);

	rc = MQTT_SUCCESS;
	return rc;
}
