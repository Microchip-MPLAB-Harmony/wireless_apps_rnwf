/*******************************************************************************
* Copyright (C) 2020 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/

/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include <string.h>
#include <stdio.h>
#include <stddef.h> 
#include <stdbool.h>
#include <stdlib.h>

/* This section lists the other files that are included in this file.*/
#include "app_rnwf02.h"
#include "user.h"
#include "definitions.h"
#include "configuration.h"
#include "system/debug/sys_debug.h"
#include "system/wifi/sys_rnwf_wifi_service.h"
#include "system/inf/sys_rnwf_interface.h"
#include "system/net/sys_rnwf_net_service.h"
#include "system/sys_rnwf_system_service.h"

/* Variable to check the UART transfer */
static volatile bool g_isUARTTxComplete = true;

/*Shows the he application's current state*/
static APP_DATA g_appData;

/*Application buffer to store data*/
static uint8_t g_appBuf[SYS_RNWF_IF_LEN_MAX];

/* TCP Socket Configurations*/
SYS_RNWF_NET_SOCKET_t g_tcpClientSocket = {
    .bind_type = SYS_RNWF_NET_BIND_TYPE0,
    .sock_port = SYS_RNWF_NET_SOCK_PORT0,
    .sock_type = SYS_RNWF_NET_SOCK_TYPE0,
    .sock_addr = SYS_RNWF_NET_SOCK_ADDR0,
    .IP        = SYS_RNWF_NET_IPV4,
};

/* DMAC Channel Handler Function */
static void APP_RNWF_usartDmaChannelHandler(DMAC_TRANSFER_EVENT event, uintptr_t contextHandle)
{
    if (event == DMAC_TRANSFER_EVENT_COMPLETE)
    {
        g_isUARTTxComplete = true;
    }
}

/* Application Wi-fi Callback Handler function */
void SYS_RNWF_WIFI_CallbackHandler(SYS_RNWF_WIFI_EVENT_t event, uint8_t *p_str)
{
            
    switch(event)
    {
        /* SNTP UP event code*/
        case SYS_RNWF_SNTP_UP:
        {            
            SYS_CONSOLE_PRINT("SNTP UP:%s\n", &p_str[2]);  
            break;
        }
        
        /* Wi-Fi connected event code*/
        case SYS_RNWF_CONNECTED:
        {
            SYS_CONSOLE_PRINT("Wi-Fi Connected    \r\n");
            break;
        }
         
        /* Wi-Fi disconnected event code*/
        case SYS_RNWF_DISCONNECTED:
        {
            SYS_CONSOLE_PRINT("Wi-Fi Disconnected\nReconnecting... \r\n");
            SYS_CONSOLE_PRINT("Connecting to server\r\n");
            SYS_RNWF_WIFI_SrvCtrl(SYS_RNWF_STA_CONNECT, NULL);
            break;
        }
        
        /* Wi-Fi DHCP complete event code*/
        case SYS_RNWF_IPv4_DHCP_DONE:
        {
            SYS_CONSOLE_PRINT("DHCP Done...%s \r\n",&p_str[2]); 
            if(SYS_RNWF_NET_SOCK_TYPE_IPv4_0 == SYS_RNWF_NET_IPV4)
             {    
                SYS_RNWF_NET_SockSrvCtrl(SYS_RNWF_NET_SOCK_TCP_OPEN, &g_tcpClientSocket);
                 
             }
            break;
        }
        
        /* Wi-Fi IPv6 DHCP complete event code*/
        case SYS_RNWF_IPv6_DHCP_DONE:
        {
            SYS_CONSOLE_PRINT("IPv6 DHCP Done...%s \r\n",&p_str[2]); 
//            SYS_RNWF_NET_SockSrvCtrl(SYS_RNWF_NET_SOCK_TCP_OPEN, &g_tcpClientSocket);
            if(SYS_RNWF_NET_SOCK_TYPE_IPv6_LOCAL0 != 0) 
             {   
                 /*Local IPv6 address code*/  
                
             }
            else if(SYS_RNWF_NET_SOCK_TYPE_IPv6_GLOBAL0 != 0)
             {
                 /*Global IPv6 address code*/
             }
            break;
        }
        
        /* Wi-Fi scan indication event code*/
        case SYS_RNWF_SCAN_INDICATION:
        {
            break;
        }
        
        /* Wi-Fi scan complete event code*/
        case SYS_RNWF_SCAN_DONE:
        {
            break;
        }
        
        default:
            break;
                    
    }    
}

/* Application NET socket Callback Handler function */
void SYS_RNWF_NET_SockCallbackHandler(uint32_t socket, SYS_RNWF_NET_SOCK_EVENT_t event, uint8_t *p_str)
{
    switch(event)
    {
        /* Net socket connected event code*/
        case SYS_RNWF_NET_SOCK_EVENT_CONNECTED:    
        {
            SYS_CONSOLE_PRINT("Connected to Server!\r\n" );
            break;
        }
          
        /* Net socket disconnected event code*/
        case SYS_RNWF_NET_SOCK_EVENT_DISCONNECTED:
        {
            SYS_CONSOLE_PRINT("DisConnected!\r\n");
            SYS_RNWF_NET_SockSrvCtrl(SYS_RNWF_NET_SOCK_CLOSE, &socket);
            break;
        }
         
        /* Net socket error event code*/
        case SYS_RNWF_NET_SOCK_EVENT_ERROR:
        {
            SYS_CONSOLE_PRINT("ERROR : %s\r\n",p_str);
            break;
        }
            
        /* Net socket read event code*/
        case SYS_RNWF_NET_SOCK_EVENT_READ:
        {         
//            uint8_t rx_data[64];
            uint8_t rx_data[1024];
            int32_t rcvd_len;
            uint16_t rx_len = *(uint16_t *)p_str;
            memset(rx_data,0,1024);
           
            if((rx_len < 1024) && (rcvd_len = SYS_RNWF_NET_TcpSockRead(socket, rx_len, rx_data)) > 0)
            {
                rx_data[rx_len] = '\n';
//                SYS_CONSOLE_PRINT("Rx->%s %ld\r\n", rx_data,sizeof(rx_data));
                for(int i=0;rx_data[i];i++)
                    SYS_CONSOLE_PRINT("%c", rx_data[i]);
                SYS_CONSOLE_PRINT("\r\n");
                SYS_RNWF_NET_TcpSockWrite(socket, rx_len, rx_data); 
            }            
            break; 
        }
        
        default:
            break;                  
    }    
    
}

/* Application Initialization function */
void APP_RNWF02_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    g_appData.state = APP_STATE_INITIALIZE;
}

/*******************************************************************************
 
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */


/* Maintain the application's state machine.*/
void APP_RNWF02_Tasks ( void )
{
    switch(g_appData.state)
    {
         /* Application's state machine's initial state. */
        case APP_STATE_INITIALIZE:
        {
            SYS_CONSOLE_PRINT("Start Of Application\r\n");
            DMAC_ChannelCallbackRegister(DMAC_CHANNEL_0, APP_RNWF_usartDmaChannelHandler, 0);
            SYS_RNWF_IF_Init();
            
            g_appData.state = APP_STATE_REGISTER_CALLBACK;
            
            break;
        }
        
        /* Register the necessary callbacks */
        case APP_STATE_REGISTER_CALLBACK:
        {
              SYS_RNWF_SYSTEM_SrvCtrl(SYS_RNWF_SYSTEM_GET_MAN_ID, g_appBuf);    
              SYS_CONSOLE_PRINT("Manufacturer = %s\r\n", g_appBuf);            
              
            /* RNWF Application Callback register */
            SYS_RNWF_WIFI_SrvCtrl(SYS_RNWF_WIFI_SET_CALLBACK, SYS_RNWF_WIFI_CallbackHandler);      
            SYS_RNWF_NET_SockSrvCtrl(SYS_RNWF_NET_SOCK_SET_CALLBACK, SYS_RNWF_NET_SockCallbackHandler);
          
            /* Wi-Fi Connectivity */
            SYS_RNWF_WIFI_PARAM_t wifi_sta_cfg = {SYS_RNWF_WIFI_MODE_STA, SYS_RNWF_WIFI_STA_SSID, SYS_RNWF_WIFI_STA_PWD, SYS_RNWF_STA_SECURITY, SYS_RNWF_WIFI_STA_AUTOCONNECT};        
            SYS_RNWF_WIFI_SrvCtrl(SYS_RNWF_SET_WIFI_PARAMS, &wifi_sta_cfg);

            g_appData.state = APP_STATE_TASK;
            break;
        }
        
        /* Run Event handler */
        case APP_STATE_TASK:
        {
            SYS_RNWF_IF_EventHandler();
            break;
        }
        default:
        {
            break;
        }
    }
}
