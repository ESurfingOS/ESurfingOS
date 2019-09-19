#include "ctiot_mqtt_demo.h"
#include "los_base.h"
#include "los_task.ph"
#include "los_typedef.h"
#include "los_sys.h"
#include "ctiot_mqtt_client.h"
#include "osdepends/atiny_osdep.h"
#include "log/atiny_log.h"
#include "cJSON.h"

#include "ctiot_device_info.h"


#define MAX_LEN  2048

#ifndef array_size
#define array_size(a) (sizeof(a)/sizeof(*(a)))
#endif

mqtt_client_s *g_phandle = NULL;

#include <string.h>


//周期上报数据
void ctiot_mqtt_demo_data_report(void)
{
    int cnt = 0;

	(void)LOS_TaskDelay(1000);
	
    while(1)
    {

        if(g_phandle)
        {
			char* payload = NULL;
			payload="{\"test_key\":\"test_payload\"}";
			CTIOT_MSG_STATUS result = CTIOT_SUCCESS;
			result = ctiot_mqtt_msg_publish("tr_up",0,payload);
			ATINY_LOG(LOG_INFO, "ctiot_mqtt_msg_publish result : %d", result);

        }
        else
        {
            ATINY_LOG(LOG_ERR, "g_phandle null");
        }
        (void)LOS_TaskDelay(10 * 1000);
    }
}

UINT32 ctiot_mqtt_create_report_task()
{
    UINT32 uwRet = LOS_OK;
    TSK_INIT_PARAM_S task_init_param;
    UINT32 TskHandle;

    task_init_param.usTaskPrio = 1;
    task_init_param.pcName = "ctiont_mqtt_demo_data_report";
    task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)ctiot_mqtt_demo_data_report;
    task_init_param.uwStackSize = 0x1000;

    uwRet = LOS_TaskCreate(&TskHandle, &task_init_param);
    if(LOS_OK != uwRet)
    {
        return uwRet;
    }
    return uwRet;
}
void ctiot_mqtt_tr_cb(void *p)
{	
	char *payload = (char *)p;

	ATINY_LOG(LOG_DEBUG, "cmd dn data: %s payload!",payload);
}

CTIOT_CB_FUNC ctiotCbFunc;

void ctiot_mqtt_demo_entry(void)
{
    UINT32 uwRet = LOS_OK;
    mqtt_param_s ctiot_params;
    mqtt_device_info_s device_info;

    ctiot_params.server_ip = DEVICE_TCPADDRESS;
    ctiot_params.server_port = DEVICE_TCPPORT;

    ctiot_params.info.security_type = MQTT_SECURITY_TYPE_NONE;

	ctiotCbFunc.ctiot_mqtt_cmd_dn_tr = ctiot_mqtt_tr_cb;


    if(ATINY_OK != ctiot_mqtt_init(&ctiot_params, &ctiotCbFunc, &g_phandle))
    {
        return;
    }

    uwRet = ctiot_mqtt_create_report_task();
    if(LOS_OK != uwRet)
    {
        return;
    }

    device_info.codec_mode = MQTT_CODEC_MODE_JSON;
    device_info.password = DEVICE_TOKEN;
    device_info.connection_type = MQTT_STATIC_CONNECT;
    device_info.u.s_info.deviceid  = DEVICE_ID;

	ATINY_LOG(LOG_ERR, "deviceID: %s , password: %s ",device_info.u.s_info.deviceid,device_info.password);
    (void)ctiot_mqtt_login	(&device_info, g_phandle);
    return ;
}
