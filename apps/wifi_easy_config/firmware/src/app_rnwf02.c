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
#include "system/inf/sys_rnwf_interface.h"
#include "system/net/sys_rnwf_net_service.h"
#include "system/sys_rnwf_system_service.h"
#include "system/wifi/sys_rnwf_wifi_service.h"
#include "system/wifiprov/sys_rnwf_provision_service.h"


// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

/*Shows the application's current state*/
static APP_DATA g_appData;


/* Variable to check the UART transfer */
static volatile bool g_isUARTTxComplete = true;

/*Application buffer to store data*/
static uint8_t g_appBuf[SYS_RNWF_IF_LEN_MAX];
// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


/* DMAC Channel Handler Function */
static void APP_RNWF_usartDmaChannelHandler ( DMAC_TRANSFER_EVENT event, uintptr_t contextHandle)
{
    if (event == DMAC_TRANSFER_EVENT_COMPLETE)
    {
        g_isUARTTxComplete = true;
    }
}

/* Application Wifi Callback Handler function */
static void SYS_RNWF_WIFI_CallbackHandler ( SYS_RNWF_WIFI_EVENT_t event, SYS_RNWF_WIFI_HANDLE_t wifiHandler)
{
    uint8_t *p_str = (uint8_t *)wifiHandler;
    switch(event)
    {   
        /* Wi-Fi connected event code*/
        case SYS_RNWF_WIFI_CONNECTED:
        {
            SYS_CONSOLE_PRINT("Wi-Fi Connected    \r\n");
            break;
        }
        
        /* Wi-Fi disconnected event code*/
        case SYS_RNWF_WIFI_DISCONNECTED:
        {
            SYS_CONSOLE_PRINT("Wi-Fi Disconnected\nReconnecting... \r\n");
            SYS_RNWF_WIFI_SrvCtrl(SYS_RNWF_WIFI_STA_CONNECT, NULL);
            break;
        }
        
        /* Wi-Fi DHCP complete event code*/
        case SYS_RNWF_WIFI_DHCP_IPV4_COMPLETE:
        {
            SYS_CONSOLE_PRINT("IPv4 DHCP Done...%s \r\n",&p_str[2]); 
            break;
        }
        
        /* Wi-Fi IPv6 Local DHCP complete event code*/
        case SYS_RNWF_WIFI_DHCP_IPV6_LOCAL_COMPLETE:
        {
            SYS_CONSOLE_PRINT("IPv6 Local DHCP Done...%s \r\n",&p_str[2]); 
            
            /*Local IPv6 address code*/     
            break;
        }
        
        /* Wi-Fi IPv6 Global DHCP complete event code*/
        case SYS_RNWF_WIFI_DHCP_IPV6_GLOBAL_COMPLETE:
        {
            SYS_CONSOLE_PRINT("IPv6 Global DHCP Done...%s \r\n",&p_str[2]); 
            
            /*Global IPv6 address code*/     
            break;
        }
        
        default:
        {
            break;   
        }
    }    
}

/* Application Wifi Provision Callback handler */
static void SYS_RNWF_WIFIPROV_CallbackHandler ( SYS_RNWF_PROV_EVENT_t event,SYS_RNWF_WIFI_PROV_HANDLE_t wifiProvHandler)
{
    uint8_t *p_str = (uint8_t *)wifiProvHandler;
    switch(event)
    {
        /**<Provisionging complete*/
        case SYS_RNWF_PROV_COMPLTE:
        {
            SYS_RNWF_PROV_SrvCtrl(SYS_RNWF_PROV_DISABLE, NULL);
            SYS_RNWF_WIFI_SrvCtrl(SYS_RNWF_WIFI_SET_CALLBACK, SYS_RNWF_WIFI_CallbackHandler);
            
            // Application can save the configuration in NVM
            SYS_RNWF_WIFI_SrvCtrl(SYS_RNWF_SET_WIFI_PARAMS, (void *)p_str);     
            break;
        }    
        
        /**<Provisionging Failure*/
        case SYS_RNWF_PROV_FAILURE:
        {
            break;
        }
        
        default:
        {
            break;
        }
    }
    
}


/* Application Initialization function */
void APP_RNWF02_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    g_appData.state = APP_STATE_INITIALIZE;
}


/* Maintain the application's state machine. */
void APP_RNWF02_Tasks ( void )
{
    switch(g_appData.state)
    {
        /* Application's state machine's initial state. */
        case APP_STATE_INITIALIZE:
        {
            DMAC_ChannelCallbackRegister(DMAC_CHANNEL_0, APP_RNWF_usartDmaChannelHandler, 0);
            SYS_RNWF_IF_Init();
            
            g_appData.state = APP_STATE_REGISTER_CALLBACK;
            break;
        }
        
        /* Register the necessary callbacks */
        case APP_STATE_REGISTER_CALLBACK:
        {
            
            SYS_RNWF_SYSTEM_SrvCtrl(SYS_RNWF_SYSTEM_GET_MAN_ID, g_appBuf);    
            SYS_CONSOLE_PRINT("\r\nManufacturer = %s\r\n", g_appBuf);  
             
            SYS_RNWF_SYSTEM_SrvCtrl(SYS_RNWF_SYSTEM_SW_REV, g_appBuf);    
            SYS_CONSOLE_PRINT("\r\nSoftware Revision:- %s\r\n", g_appBuf);
            
            SYS_RNWF_SYSTEM_SrvCtrl(SYS_RWWF_SYSTEM_GET_WIFI_INFO, g_appBuf);    
            SYS_CONSOLE_PRINT("\r\nWi-Fi Info:- \r\n%s\r\n", g_appBuf);
            
            /* Set Regulatory domain/Country Code */
            const char *regDomain = SYS_RNWF_COUNTRYCODE;
            SYS_CONSOLE_PRINT("\r\nSetting regulatory domain : %s\r\n",regDomain);
            SYS_RNWF_WIFI_SrvCtrl(SYS_RNWF_WIFI_SET_REGULATORY_DOMAIN, (void *)regDomain);
            
            // Enable Provisioning Mode
            SYS_RNWF_PROV_SrvCtrl(SYS_RNWF_PROV_ENABLE, NULL);
            SYS_RNWF_PROV_SrvCtrl(SYS_RNWF_PROV_SET_CALLBACK, (void *)SYS_RNWF_WIFIPROV_CallbackHandler);
            
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

/*******************************************************************************
 End of File
 */
