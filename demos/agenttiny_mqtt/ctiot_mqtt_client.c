#include "ctiot_mqtt_client.h"
#include "MQTTClient.h"

#include "atiny_mqtt/mqtt_client.h"
#include "los_base.h"
#include "los_task.ph"
#include "los_typedef.h"
#include "los_sys.h"
#include "log/atiny_log.h"
#include "MQTTClient.h"

#include "cJSON.h"



typedef enum
{
	MQTT_CONNECT_WITH_PRODUCT_ID,
	MQTT_CONNECT_WITH_DEVICE_ID
}mqtt_dynamic_connect_state_e;

typedef struct
{
	mqtt_static_connection_info_s save_info;
	char *  got_password;
	mqtt_dynamic_connect_state_e state;
	uint8_t connection_update_flag;
	uint8_t has_device_id;
	uint8_t reserve[2];
}mqtt_dynamic_info_s;

struct ctiot_mqtt_client_tag_s
{
	mqtt_device_info_s device_info;
	MQTTClient client;
	mqtt_param_s params;
	CTIOT_CB_FUNC *ctiotCbFunc;
	mqtt_dynamic_info_s dynamic_info;
	char *sub_topic;
	uint8_t init_flag;
	uint8_t reserve[3];
};

struct ctiot_mqtt_client_tag_s *m_mqtt_client; 

void ctiot_mqtt_client_init(void)
{
	m_mqtt_client = mqtt_get_client(); 
}

static void ctiot_cmd_dn_execcmd_entry(MessageData *md) 
{ 
	if ((md == NULL) || (md->message == NULL) || (mqtt_modify_payload(md) != ATINY_OK)) 
	{ 
		ATINY_LOG(LOG_FATAL, "null point"); 
		return; 
	} 
 
	char *payload = md->message->payload; 
 
	CMD_DN_EXECCMD para = { 0 }; 
 
	if (m_mqtt_client->ctiotCbFunc != NULL && m_mqtt_client->ctiotCbFunc->ctiot_cmd_dn_execcmd != NULL) 
	{ 
		CTIOT_MQTT_PARA paraHead = ctiot_json_parsing(payload); 
		int i = 0; 
		while (i < paraHead.count) 
		{ 
			char* paraname = paraHead.paraList[i].paraName; 
			if (strcmp(paraname, "taskId") == 0) 
			{ 
				para.taskId = (int)paraHead.paraList[i].u.ctiotInt.val; 
			} 
			else if(strcmp(paraname,"af")==0)
			{
				para.af = (int)paraHead.paraList[i].u.ctiotDouble.val;
			}
			else if(strcmp(paraname,"at")==0)
			{
				para.at = (double)paraHead.paraList[i].u.ctiotDouble.val;
			}
 
			i++; 
		} 
		m_mqtt_client->ctiotCbFunc->ctiot_cmd_dn_execcmd(&para); 
	}
}

CTIOT_MSG_STATUS ctiot_data_report_datarep(DATA_REPORT_DATAREP* para)
{
	CTIOT_MSG_STATUS ret = CTIOT_SUCCESS;
	CTIOT_MQTT_PARA  mqttPara = { 0 };
	mqttPara.count = 2;
	mqttPara.paraList[0].ctiotParaType = CTIOT_INT;
	mqttPara.paraList[0].paraName = "af";
	mqttPara.paraList[0].u.ctiotInt.min = 2;
	mqttPara.paraList[0].u.ctiotInt.max = 3;
	mqttPara.paraList[0].u.ctiotInt.val = para->af;

	mqttPara.paraList[1].ctiotParaType = CTIOT_DOUBLE;
	mqttPara.paraList[1].paraName = "at";
	mqttPara.paraList[1].u.ctiotDouble.min = 2.000000;
	mqttPara.paraList[1].u.ctiotDouble.max = 3.000000;
	mqttPara.paraList[1].u.ctiotDouble.val = para->at;


	ret = ctiot_mqtt_validate(mqttPara);
	if (ret != CTIOT_SUCCESS)
	{
		return ret;
	}

	ret = ctiot_msg_pub("datarep", para->qos, mqttPara);;

	return ret;
}
CTIOT_MSG_STATUS ctiot_cmd_response_cmdreponse(CMD_RESPONSE_CMDREPONSE* para)
{
	CTIOT_MSG_STATUS ret = CTIOT_SUCCESS;
	CTIOT_MQTT_PARA  mqttPara = { 0 };
	mqttPara.count = 2;
	mqttPara.paraList[0].ctiotParaType = CTIOT_INT;
	mqttPara.paraList[0].paraName = "af";
	mqttPara.paraList[0].u.ctiotInt.min = 2;
	mqttPara.paraList[0].u.ctiotInt.max = 3;
	mqttPara.paraList[0].u.ctiotInt.val = para->af;

	mqttPara.paraList[1].ctiotParaType = CTIOT_DOUBLE;
	mqttPara.paraList[1].paraName = "at";
	mqttPara.paraList[1].u.ctiotDouble.min = 2.000000;
	mqttPara.paraList[1].u.ctiotDouble.max = 3.000000;
	mqttPara.paraList[1].u.ctiotDouble.val = para->at;


	ret = ctiot_mqtt_validate(mqttPara);
	if (ret != CTIOT_SUCCESS)
	{
		return ret;
	}

	ret = ctiot_msg_response("cmdreponse", para->qos, para->taskId, mqttPara);

	return ret;
}

typedef void (* CTIOT_CB)(MessageData *md);

int ctiot_mqtt_subscribe (void* mhandle) 
{ 
	struct ctiot_mqtt_client_tag_s *handle = mhandle;
	char *topic; 
	CTIOT_CB topic_callback = NULL; 
	int rc; 
	
	if (handle->sub_topic) 
	{ 
		(void)MQTTSetMessageHandler(&handle->client, handle->sub_topic, NULL); 
		atiny_free(handle->sub_topic); 
		handle->sub_topic = NULL; 
	} 
	topic = "execcmd";
	topic_callback = ctiot_cmd_dn_execcmd_entry;
	(void)MQTTSetMessageHandler(&handle->client, topic, topic_callback);
	ATINY_LOG(LOG_INFO, "subcribe static topic: %s", topic);

	rc = MQTT_SUCCESS; 
	return rc;
}
