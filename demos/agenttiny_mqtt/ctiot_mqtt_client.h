#ifndef _CTIOT_MQTT_CLIENT_H
#define _CTIOT_MQTT_CLIENT_H

#include "atiny_mqtt/mqtt_client.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
	int taskId;
	int af;
	double at;
} CMD_DN_EXECCMD; 

typedef struct {
	mqtt_qos_e qos;
	int af;
	double at;
} DATA_REPORT_DATAREP; 

typedef struct {
	mqtt_qos_e qos;
	int taskId;
	int af;
	double at;
} CMD_RESPONSE_CMDREPONSE; 



CTIOT_MSG_STATUS ctiot_data_report_datarep(DATA_REPORT_DATAREP* para);

CTIOT_MSG_STATUS ctiot_cmd_response_cmdreponse(CMD_RESPONSE_CMDREPONSE* para);

typedef struct{
	void(*ctiot_cmd_dn_tr)(CMD_DN_EXECCMD*);
}CTIOT_CB_FUNC; 

int ctiot_mqtt_subscribe(void* mhandle);

void ctiot_mqtt_client_init(void);
#ifdef __cplusplus
}
#endif
#endif //_CTIOT_MQTT_CLIENT_H
