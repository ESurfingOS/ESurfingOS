/*----------------------------------------------------------------------------
 * Copyright (c) <2016-2018>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations, which might
 * include those applicable to Huawei LiteOS of U.S. and the country in which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance with such
 * applicable export control laws and regulations.
 *---------------------------------------------------------------------------*/

#include "sys_init.h"
#ifdef CONFIG_FEATURE_FOTA
#include "ota_port.h"
#endif

#ifdef WITH_MQTT
#include "ctiot_mqtt_demo.h"
#endif


static UINT32 g_atiny_tskHandle;
static UINT32 g_fs_tskHandle;




extern void ctiot_mqtt_demo_entry();

void atiny_task_entry(void)
{
#ifdef WITH_LWIP
    hieth_hw_init();
    net_init();
#endif
		
    ctiot_mqtt_demo_entry();
}


UINT32 creat_agenttiny_task(VOID)
{
    UINT32 uwRet = LOS_OK;
    TSK_INIT_PARAM_S task_init_param;

    task_init_param.usTaskPrio = 2;
    task_init_param.pcName = "agenttiny_task";
    task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)atiny_task_entry;

#if defined(CONFIG_FEATURE_FOTA) || defined(WITH_MQTT)
    task_init_param.uwStackSize = 0x2000; /* fota use mbedtls bignum to verify signature  consuming more stack  */
#else
    task_init_param.uwStackSize = 0x1000;
#endif

    uwRet = LOS_TaskCreate(&g_atiny_tskHandle, &task_init_param);
    if(LOS_OK != uwRet)
    {
        return uwRet;
    }
    return uwRet;
}






#if defined(WITH_DTLS) && defined(SUPPORT_DTLS_SRV)
//static UINT32 g_dtls_server_tskHandle;
//uint32_t create_dtls_server_task()
//{
//    uint32_t uwRet = LOS_OK;
//    TSK_INIT_PARAM_S task_init_param;

//    task_init_param.usTaskPrio = 3;
//    task_init_param.pcName = "dtls_server_task";
//    extern void dtls_server_task(void);
//    task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)dtls_server_task;

//    task_init_param.uwStackSize = 0x1000;

//    uwRet = LOS_TaskCreate(&g_dtls_server_tskHandle, &task_init_param);
//    if(LOS_OK != uwRet)
//    {
//        return uwRet;
//    }
//    return uwRet;
//}
#endif


UINT32 create_work_tasks(VOID)
{
    UINT32 uwRet = LOS_OK;

    uwRet = creat_agenttiny_task();
    if (uwRet != LOS_OK)
    {
    	return LOS_NOK;
    }

//#if defined(USE_PPPOS)
//    #include "osport.h"
//    extern void uart_init(void);  //this uart used for the pppos interface
//    uart_init();
//    extern VOID *main_ppp(UINT32  args);
//    task_create("main_ppp", main_ppp, 0x1500, NULL, NULL, 2);
//#endif


//#if defined(WITH_DTLS) && defined(SUPPORT_DTLS_SRV)
//    uwRet = create_dtls_server_task()
//    if (uwRet != LOS_OK)
//    {
//    	return LOS_NOK;
//    }
//#endif

    return uwRet;

}



