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
#include "dhcp.h"
#include "lwip/dns.h"
#include "lwip/prot/dhcp.h"


#ifdef WITH_LWIP
struct     netif gnetif;

ip4_addr_t ipaddr;
ip4_addr_t netmask;
ip4_addr_t gw;

ip4_addr_t DNSServerAddr;

int8_t os_dhcp(void);
int8_t os_dns(void);


static void lwip_impl_register(void)
{
    STlwIPFuncSsp stlwIPSspCbk = {0};
    stlwIPSspCbk.pfRand = hal_rng_generate_number;
    lwIPRegSspCbk(&stlwIPSspCbk);
}

int8_t net_init(void)
{
    IP4_ADDR(&ipaddr, 0, 0, 0, 0);
    IP4_ADDR(&netmask, 0, 0, 0, 0);
    IP4_ADDR(&gw, 0, 0, 0, 0);

    lwip_impl_register();
    
    /*register ethernet api*/
    ethernetif_api_register(&g_eth_api);
	
	  /* Initilialize the LwIP stack with RTOS(creat a tcp/ip thread) */
    tcpip_init(NULL, NULL);
    
		/* add the network interface (IPv4/IPv6) with RTOS */
	  netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, ethernetif_init, tcpip_input);
	
		while( !netif_is_link_up(&gnetif) )
		{
			printf("add netif fail\r\n");
			
			netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, ethernetif_init, tcpip_input);
			LOS_TaskDelay(1000);
		}
		
    /* Registers the default network interface */
    netif_set_default(&gnetif);
		
    /* setup netif. When the netif is fully configured this function must be called */
    netif_set_up(&gnetif);
			
	  os_dhcp();
			
		return 0;
}

void print_dhcp_state(uint8_t state)
{
	switch(state)
	{
		case DHCP_STATE_OFF:
			printf("dhcp state is \"DHCP_STATE_OFF\"\r\n");
		break;
		
		case DHCP_STATE_REQUESTING:
			printf("dhcp state is \"DHCP_STATE_REQUESTING\"\r\n");
		break;
						
		case DHCP_STATE_INIT:
			printf("dhcp state is \"DHCP_STATE_INIT\"\r\n");
		break;
		
		case DHCP_STATE_REBOOTING:
			printf("dhcp state is \"DHCP_STATE_REBOOTING\"\r\n");
		break;
		
		case DHCP_STATE_REBINDING:
			printf("dhcp state is \"DHCP_STATE_REBINDING\"\r\n");
		break;
						
		case DHCP_STATE_RENEWING:
			printf("dhcp state is \"DHCP_STATE_RENEWING\"\r\n");
		break;
		
		case DHCP_STATE_SELECTING:
			printf("dhcp state is \"DHCP_STATE_SELECTING\"\r\n");
		break;
		
		case DHCP_STATE_INFORMING:
			printf("dhcp state is \"DHCP_STATE_INFORMING\"\r\n");
		break;
		
		case DHCP_STATE_CHECKING:
			printf("dhcp state is \"DHCP_STATE_CHECKING\"\r\n");
		break;
						
		case DHCP_STATE_BOUND:
			printf("dhcp state is \"DHCP_STATE_BOUND\"\r\n");
		break;
		
		case DHCP_STATE_BACKING_OFF:
			printf("dhcp state is \"DHCP_STATE_BACKING_OFF\"\r\n");
		break;
		
		default:
			break;
	}
}
int8_t os_dhcp(void)
{
	struct dhcp *dhcp;
	//dns_setserver(0, &ipaddr);
	
	if( dhcp_start(&gnetif) == ERR_OK )
	{
		printf("start dhcp success\r\n");
		
		while(1)
		{
			if( dhcp_supplied_address(&gnetif) )
			{
		    printf("dhcp client get ip success\r\n");
				dhcp = netif_dhcp_data(&gnetif);
				print_dhcp_state(dhcp->state);
				break;
			}
			dhcp = netif_dhcp_data(&gnetif);
			print_dhcp_state(dhcp->state);
			LOS_TaskDelay(1000);
		}

		printf("ipaddr: %d.%d.%d.%d\r\n", ip4_addr1(&dhcp->offered_ip_addr), ip4_addr2(&dhcp->offered_ip_addr), ip4_addr3(&dhcp->offered_ip_addr), ip4_addr4(&dhcp->offered_ip_addr));
		printf("netmask: %d.%d.%d.%d\r\n", ip4_addr1(&dhcp->offered_sn_mask), ip4_addr2(&dhcp->offered_sn_mask), ip4_addr3(&dhcp->offered_sn_mask), ip4_addr4(&dhcp->offered_sn_mask));
		printf("gw: %d.%d.%d.%d\r\n", ip4_addr1(&dhcp->offered_gw_addr), ip4_addr2(&dhcp->offered_gw_addr), ip4_addr3(&dhcp->offered_gw_addr), ip4_addr4(&dhcp->offered_gw_addr));
	
		os_dns();
	}
	else
	{
		printf("start dhcp fail\r\n");
	}
	
	return 0;
}


/****************************************** dns ********************************************/
static uint8_t os_dns_state = 0;

void os_dns_callback(const char *name, const ip_addr_t *ipaddr, void *callback_arg)
{
	printf("dns server name:%s\r\n",name);
	
	printf("dns server ip:%d.%d.%d.%d\r\n", ip4_addr1(ipaddr), ip4_addr2(ipaddr), ip4_addr3(ipaddr), ip4_addr4(ipaddr));
	
	os_dns_state = 1;
}

int8_t os_dns(void)
{
	ip4_addr_t serveripaddr;
	
	IP4_ADDR(&DNSServerAddr, 114, 114, 114, 114);
	
	dns_setserver(0, &DNSServerAddr);
	
	printf("start dns...\r\n");
	dns_init();
	
	dns_gethostbyname("www.baidu.com", &serveripaddr, os_dns_callback, NULL);
	
	while(1)
	{
		if( os_dns_state == 1 )
			break;
		
		LOS_TaskDelay(1000);
	}
	
	printf("dns success\r\n");
	
	return 0;
}

#endif



uint32_t HAL_GetTick(void)
{
    return (uint32_t)LOS_TickCountGet();
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    while(1)
    {
    }
    /* USER CODE END Error_Handler_Debug */
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Configure the main internal regulator output voltage
    */
    __HAL_RCC_PWR_CLK_ENABLE();

    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = 16;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 180;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        _Error_Handler(__FILE__, __LINE__);
    }

    /**Activate the Over-Drive mode
    */
    if (HAL_PWREx_EnableOverDrive() != HAL_OK)
    {
        _Error_Handler(__FILE__, __LINE__);
    }

    /**Initializes the CPU, AHB and APB busses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
    {
        _Error_Handler(__FILE__, __LINE__);
    }

    SystemCoreClockUpdate();
}
#ifdef WITH_LWIP
void hieth_hw_init(void)
{
    extern void ETH_IRQHandler(void);
    (void)LOS_HwiCreate(ETH_IRQn, 1, 0, ETH_IRQHandler, 0);
}
#endif

/*
 * atiny_adapter user interface
 */
void atiny_usleep(unsigned long usec)
{
    delayus((uint32_t)usec);
}

int atiny_random(void *output, size_t len)
{
    return hal_rng_generate_buffer(output, len);
}

void atiny_reboot(void)
{
    HAL_NVIC_SystemReset();
}

