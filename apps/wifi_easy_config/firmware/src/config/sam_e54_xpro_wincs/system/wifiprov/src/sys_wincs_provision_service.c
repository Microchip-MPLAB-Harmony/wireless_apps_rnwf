/*******************************************************************************
  WINCS Host Assisted Wifi Provision Service Implementation

  File Name:
    sys_wincs_wifi_provision_service.c

  Summary:
    Source code for the WINCS Host Assisted Wifi Provision  Service implementation.

  Description:
    This file contains the source code for the WINCS Host Assisted Wifi Provision Service
    implementation.
 *******************************************************************************/

/*******************************************************************************
Copyright (C) 2020 released Microchip Technology Inc.  All rights reserved.

 
Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
//DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* This section lists the other files that are included in this file.
 */
#include "system/net/sys_wincs_net_service.h"
#include "system/wifi/sys_wincs_wifi_service.h"
#include "system/wifiprov/sys_wincs_provision_service.h"
#include "configuration.h"


#define SYS_WINCS_WIFI_PROV_SSID				"Rio0_SOFTAP"
#define SYS_WINCS_WIFI_PROV_PWD        			"12345678"
#define SYS_WINCS_PROV_SECURITY				SYS_WINCS_WPA2
#define SYS_WINCS_WIFI_PROV_AUTOCONNECT			1
/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */

/*  A brief description of a section can be given directly below the section
    banner.
 */

/*Provision callback handler*/
SYS_WINCS_PROV_CALLBACK_t g_provCallBackHandler; 


/* TCP Socket Configurations */
SYS_WINCS_NET_SOCKET_t g_provisionSocket = {
    /*Socket bind type*/
    .bind_type = SYS_WINCS_NET_BIND_TYPE0,
    
    /*Socket port number */
    .sock_port = SYS_WINCS_NET_SOCK_PORT0,
    
    /*Socket type(TCP/UDP)*/
    .sock_type = SYS_WINCS_NET_SOCK_TYPE0,
};


/* ************************************************************************** */
/* ************************************************************************** */
// Section: Local Functions                                                   */
/* ************************************************************************** */
/* ************************************************************************** */



/*Parse or translate security type received from mobile app*/
static SYS_WINCS_WIFI_SECURITY_t SYS_WINCS_PROV_ParseAuth(uint8_t secType)
{
    SYS_WINCS_WIFI_SECURITY_t authType = SYS_WINCS_OPEN;
    
    switch(secType)
    {
        case 1:
            authType = secType - 1;
            break;

        case 2:
            authType = secType;
            break;

        case 4:
            authType = secType + 1;
            break;
            
        default:
            SYS_WINCS_PROV_DBG_MSG("Invalid security type\r\n");
            break;
    }
    return authType;
}
            
/* Parse Wi-Fi configuration file */
/* Format is APP_WIFI_PROV_WIFI_CONFIG_ID,<SSID>,<AUTH>,<PASSPHRASE>*/
static SYS_WINCS_RESULT_t SYS_WINCS_PROV_AppParse(uint8_t *wifiCofnig, SYS_WINCS_WIFI_PARAM_t *wifi_config)
{
    char* p;    
    SYS_WINCS_RESULT_t ret = SYS_WINCS_PASS;
    
    p = strtok((char *)wifiCofnig, ",");
    if (p != NULL && !strncmp(p, SYS_WINCS_APP_WIFI_PROV_CONFIG_ID, strlen(SYS_WINCS_APP_WIFI_PROV_CONFIG_ID))) 
    {
        p = strtok(NULL, ",");
        if (p)
            wifi_config->ssid = p;

        p = strtok(NULL, ",");
        if (p) 
        {
            uint8_t security = (SYS_WINCS_WIFI_SECURITY_t)atoi(p);
             
            wifi_config->security = SYS_WINCS_PROV_ParseAuth(security);
            
            if (SYS_WINCS_OPEN < wifi_config->security &&  wifi_config->security <= SYS_WINCS_WPA3)
            {
                p = strtok(NULL, ",");
                if (p) 
                    wifi_config->passphrase =  p;
                else
                    ret = SYS_WINCS_FAIL;
            } 
            else if (wifi_config->security == SYS_WINCS_OPEN)
                wifi_config->passphrase = NULL;
            else
                ret = SYS_WINCS_FAIL;
        }
        else
            ret = SYS_WINCS_FAIL;

        wifi_config->channel = WDRV_WINC_CID_ANY;
        wifi_config->autoconnect = true;
        SYS_WINCS_PROV_DBG_MSG("Connecting to SSID:%s - PASSPHRASE:%s - AUTH:%d\r\n", 
                            wifi_config->ssid, 
                            wifi_config->passphrase, 
                            wifi_config->security
                            );
    }
    else if(p != NULL && !strncmp(p, SYS_WINCS_APP_WIFI_PROV_DONE_ID, strlen(SYS_WINCS_APP_WIFI_PROV_DONE_ID)))
    {
        
    }
    return ret;
}


/*This function processes the data received from the provision app*/
SYS_WINCS_RESULT_t SYS_RNWF_PROV_AppProcess(uint32_t socket) {
        
    SYS_WINCS_WIFI_PARAM_t wifiConfig;
    uint8_t prov_buf[SYS_WINCS_PROV_BUF_LEN_MAX];
    
    if(SYS_WINCS_NET_TcpSockRead(socket, SYS_WINCS_PROV_RECV_BUFFER_SIZE, (uint8_t *)prov_buf) > 0)
    {
        if(SYS_WINCS_PROV_AppParse(prov_buf, &wifiConfig) == SYS_WINCS_PASS)
        {
            SYS_WINCS_NET_SockSrvCtrl(SYS_WINCS_NET_SOCK_CLOSE, &socket);
            wifiConfig.mode = SYS_WINCS_WIFI_MODE_STA;
            wifiConfig.autoconnect = false;
            wifiConfig.channel = WDRV_WINC_CID_ANY;
            
            if(g_provCallBackHandler)
                g_provCallBackHandler(SYS_WINCS_PROV_COMPLTE, (uint8_t *)&wifiConfig);
            return SYS_WINCS_PASS;
        }
        else
        {
            if(g_provCallBackHandler)
                g_provCallBackHandler(SYS_WINCS_PROV_FAILURE, NULL);
        }
    }
    return SYS_WINCS_FAIL;
}

/*Provision Socket callback function*/
static void SYS_WINCS_PROV_SocketCallback(uint32_t sock, SYS_WINCS_NET_SOCK_EVENT_t event, uint8_t *p_str)
{
    switch(event)
    {
        case SYS_WINCS_NET_SOCK_EVENT_CONNECTED:
        {
            break;
        }

        case SYS_WINCS_NET_SOCK_EVENT_DISCONNECTED:
        {
            SYS_WINCS_NET_SockSrvCtrl(SYS_WINCS_NET_SOCK_CLOSE, &sock);
            break;
        }

        case SYS_WINCS_NET_SOCK_EVENT_READ:
        {
            SYS_RNWF_PROV_AppProcess(sock);
            break;
        }
        
        case SYS_WINCS_NET_SOCK_EVENT_CLIENT_CONNECTED:
        {
            SYS_WINCS_PROV_DBG_MSG("Client Socket Connected!\r\n");
            break;
        }
        
        default:
        {
            break;
        }
    }
}


/* Application Wi-fi Callback Handler function */
void SYS_WINCS_PROV_WifiCallback(SYS_WINCS_WIFI_EVENT_t event, uint8_t *p_str)
{
            
    switch(event)
    {
        /* SNTP UP event code*/
        case SYS_WINCS_SNTP_UP:
        {            
            SYS_WINCS_PROV_DBG_MSG("SNTP UP:%s\r\n", &p_str[2]);  
        }
        break;

        /* Wi-Fi connected event code*/
        case SYS_WINCS_CONNECTED:
        {
            SYS_WINCS_PROV_DBG_MSG("Wi-Fi Connected    \r\n");
            break;
        }
        
        /* Wi-Fi disconnected event code*/
        case SYS_WINCS_DISCONNECTED:
        {
            SYS_WINCS_PROV_DBG_MSG("Wi-Fi Disconnected\nReconnecting... \r\n");
            SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_STA_CONNECT, NULL);
            break;
        }
        
        /* Wi-Fi DHCP complete event code*/
        case SYS_WINCS_DHCP_DONE:
        {         
            SYS_WINCS_PROV_DBG_MSG("DHCP IPv4 : %s\r\n", p_str);
            SYS_WINCS_NET_SockSrvCtrl(SYS_WINCS_NET_SOCK_TCP_OPEN, &g_provisionSocket);
            break;
        }
        
        case SYS_WINCS_DHCP_IPV6_LOCAL_DONE:
        {
            //SYS_WINCS_PROV_DBG_MSG("[APP] : DHCP IPv6 Local : %s\r\n", p_str);
            break;
        }
        
        case SYS_WINCS_DHCP_IPV6_GLOBAL_DONE:
        {
            //SYS_WINCS_PROV_DBG_MSG("[APP] : DHCP IPv6 Global: %s\r\n", p_str);
            break;
        }
        
        /* Wi-Fi scan indication event code*/
        case SYS_WINCS_SCAN_INDICATION:
        {
            break;
        } 
        
        /* Wi-Fi scan complete event code*/
        case SYS_WINCS_SCAN_DONE:
        {
            break;
        }
        
        default:
        {
            break;
        }
                    
    }    
}


void SYS_WINCS_PROV_NetCallback(SYS_WINCS_NET_DHCP_EVENT_t event, uint8_t *p_str)
{
    switch(event)
    {
        case SYS_WINCS_NET_STA_DHCP_DONE:
        {
            SYS_WINCS_PROV_DBG_MSG("STA Connected -> DHCP IP : %s\r\n", p_str);
            SYS_WINCS_NET_SockSrvCtrl(SYS_WINCS_NET_SOCK_TCP_OPEN, &g_provisionSocket);
            break;
        }
    }
    return;
}


/*Provision Service control function*/
SYS_WINCS_RESULT_t SYS_WINCS_PROV_SrvCtrl(SYS_WINCS_PROV_SERVICE_t request, void *input)  {
    
    switch(request)
    {
        case SYS_WINCS_PROV_ENABLE:
        {                   
            
            /* WINCS Application Callback register */
            SYS_WINCS_NET_SockSrvCtrl(SYS_WINCS_NET_SOCK_SET_SRVC_CALLBACK, SYS_WINCS_PROV_NetCallback);
            SYS_WINCS_NET_SockSrvCtrl(SYS_WINCS_NET_SOCK_SET_CALLBACK, SYS_WINCS_PROV_SocketCallback);

            const char *dhcps_cfg[] = {SYS_WINCS_WIFI_AP_IP_ADDR, SYS_WINCS_WIFI_AP_IP_POOL_START};
            SYS_WINCS_NET_SockSrvCtrl(SYS_WINCS_NET_DHCP_SERVER_ENABLE, dhcps_cfg);  

            /* Wi-Fi Connectivity */
            SYS_WINCS_WIFI_PARAM_t wifi_ap_cfg = {
                .mode = SYS_WINCS_WIFI_MODE_AP,
                .ssid = SYS_WINCS_WIFI_PROV_SSID, 
                .passphrase = SYS_WINCS_WIFI_PROV_PWD,
                .security = SYS_WINCS_PROV_SECURITY,
                .channel  = WDRV_WINC_CID_2_4G_CH1,
                .ssidVisibility = true};   
            
            
            SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_SET_PARAMS, &wifi_ap_cfg);
            SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_AP_ENABLE, NULL);
            
            
            SYS_WINCS_PROV_DBG_MSG("Provision mode enabled\r\n");
            SYS_WINCS_PROV_DBG_MSG("Connect to SSID : %s   ,PASSPHRASE : %s ,AUTH : %d\r\n", 
                                wifi_ap_cfg.ssid, 
                                wifi_ap_cfg.passphrase,
                                wifi_ap_cfg.security);
            
            break;
        }
        
        
        case SYS_WINCS_PROV_DISABLE:
        {
            /* WINCS Application Callback register */
            SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_SET_SRVC_CALLBACK, NULL);
            SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_AP_DISABLE,NULL);
            
            SYS_WINCS_NET_SockSrvCtrl(SYS_WINCS_NET_SOCK_SET_SRVC_CALLBACK, NULL);
            SYS_WINCS_NET_SockSrvCtrl(SYS_WINCS_NET_DHCP_SERVER_DISABLE, NULL);
            break;
        }
       
        case SYS_WINCS_PROV_SET_CALLBACK:
        {
            if(input != NULL)
                g_provCallBackHandler = (SYS_WINCS_PROV_CALLBACK_t)input;
            break;
        }    
            
        default:
        {
            break;
        }
    }
    
    return SYS_WINCS_PASS;
}


/*Provision service init function*/
SYS_WINCS_RESULT_t SYS_WINCS_PROV_SrvInit(SYS_WINCS_PROV_MODE_t provMode)  
{
    if(provMode == SYS_WINCS_PROV_MOBILE_APP)
    {
    }
    return SYS_WINCS_PASS;
}
/* *****************************************************************************
 End of File
 */
