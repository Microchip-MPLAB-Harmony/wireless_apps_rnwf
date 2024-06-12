/*******************************************************************************
  WINCS Host Assisted MQTT Service Implementation(WINCS02)

  File Name:
    sys_wincs_mqtt_service.c

  Summary:
    Source code for the WINCS Host Assisted MQTT Service implementation.

  Description:
    This file contains the source code for the WINCS Host Assisted MQTT Service
    implementation.
 *******************************************************************************/

//DOM-IGNORE-BEGIN
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
/* This section lists the other files that are included in this file.
 */
#include <stdio.h>
#include <string.h>

/* This section lists the other files that are included in this file.
 */
#include "system/mqtt/sys_wincs_mqtt_service.h"
#include "system/net/sys_wincs_net_service.h"
#include "system/sys_wincs_system_service.h"

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */

static SYS_WINCS_MQTT_CALLBACK_t g_MqttCallBackHandler[SYS_WINCS_MQTT_SERVICE_CB_MAX] = {NULL, NULL};
    

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Local Functions                                                   */
/* ************************************************************************** */
/* ************************************************************************** */

static void SYS_WINCS_MQTT_ConnCallback
(
    DRV_HANDLE handle, 
    uintptr_t userCtx, 
    WDRV_WINC_MQTT_CONN_STATUS_TYPE state
)
{
    SYS_WINCS_MQTT_CALLBACK_t mqtt_cb_func = g_MqttCallBackHandler[1];
    switch(state)
    {
        case WDRV_WINC_MQTT_CONN_STATUS_CONNECTED:
        {
            mqtt_cb_func(SYS_WINCS_MQTT_CONNECTED, NULL);
            break;
        }
        
        case WDRV_WINC_MQTT_CONN_STATUS_CONNECTING:
        {
            SYS_WINCS_MQTT_DBG_MSG( "MQTT connecting.....\r\n");
            break;
        }
        
        case WDRV_WINC_MQTT_CONN_STATUS_DISCONNECTED:
        {
            mqtt_cb_func(SYS_WINCS_MQTT_DISCONNECTED, NULL);
            break;
        }
        
        case WDRV_WINC_MQTT_CONN_STATUS_DISCONNECTING:
        {
            SYS_WINCS_MQTT_DBG_MSG( "MQTT disconnecting.....\r\n");
            break;
        }
        
         case WDRV_WINC_MQTT_CONN_STATUS_UNKNOWN:
        {
            SYS_WINCS_MQTT_DBG_MSG( "MQTT connect status unknown.\r\n");
            break;
        }
    }
}


void SYS_WINCS_MQTT_PublishCallback
(
    DRV_HANDLE handle, 
    uintptr_t userCtx, 
    WDRV_WINC_MQTT_PUB_HANDLE pubHandle, 
    uint16_t packetId, 
    WDRV_WINC_MQTT_PUB_STATUS_TYPE status
)
{
    SYS_WINCS_MQTT_CALLBACK_t mqtt_cb_func = g_MqttCallBackHandler[1];
    switch(status)
    {
        case WDRV_WINC_MQTT_PUB_STATUS_SENT:
        {
            mqtt_cb_func(SYS_WINCS_MQTT_PUBLISH_ACK, NULL );
            break;
        }
        
        case WDRV_WINC_MQTT_PUB_STATUS_RECV:
        {
            mqtt_cb_func(SYS_WINCS_MQTT_PUBLISH_MSG_RECV, NULL );
            break;
        }
        
        case WDRV_WINC_MQTT_PUB_STATUS_ERROR:
        {
            mqtt_cb_func(SYS_WINCS_MQTT_ERROR, NULL );
            break;
        }
        default:
            break;
    }
}



void SYS_WINCS_MQTT_SubscribeCallback
(
    DRV_HANDLE handle, 
    uintptr_t userCtx, 
    const WDRV_WINC_MQTT_MSG_INFO *pMsgInfo, 
    const char *pTopicName, 
    const uint8_t *pTopicData, 
    size_t topicDataLen, 
    WDRV_WINC_MQTT_SUB_STATUS_TYPE status
)
{
    SYS_WINCS_MQTT_CALLBACK_t mqtt_cb_func = g_MqttCallBackHandler[1];
    switch(status)
    {
        case WDRV_WINC_MQTT_SUB_STATUS_ACKED:
        {
            mqtt_cb_func(SYS_WINCS_MQTT_SUBCRIBE_ACK, NULL );
            break;
        }
        
        case WDRV_WINC_MQTT_SUB_STATUS_RXDATA:
        {
            mqtt_cb_func(SYS_WINCS_MQTT_SUBCRIBE_MSG, (uint8_t *)pTopicData );
            break;
        }
        
        case WDRV_WINC_MQTT_SUB_STATUS_ERROR:
        {
            mqtt_cb_func(SYS_WINCS_MQTT_ERROR, NULL );
            break;
        }
        
        case WDRV_WINC_MQTT_SUB_STATUS_END:
        {
            mqtt_cb_func(SYS_WINCS_MQTT_UNSUBSCRIBED, NULL );
            break;
        }
        
        default:
            break;
    }
}



/*MQTT Service control function*/
SYS_WINCS_RESULT_t SYS_WINCS_MQTT_SrvCtrl
(
	 SYS_WINCS_MQTT_SERVICE_t request, 
	 void *input
)  
{
    DRV_HANDLE wdrvHandle = DRV_HANDLE_INVALID;
    
    SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_GET_DRV_HANDLE,&wdrvHandle);
    WDRV_WINC_STATUS status = WDRV_WINC_STATUS_OK;
    
    switch(request)
    {
        /**<Configure the MQTT Broker parameters*/
        case SYS_WINCS_MQTT_CONFIG:
        {
            SYS_WINCS_MQTT_CFG_t *mqtt_cfg = (SYS_WINCS_MQTT_CFG_t *)input;  
            
            WDRV_WINC_TLS_HANDLE tlsHandle = 0;
            if(mqtt_cfg->tls_idx != 0)
            {
                SYS_WINCS_NET_SockSrvCtrl(SYS_WINCS_NET_OPEN_TLS_CTX,(void *)&tlsHandle);
            }
            
            status = WDRV_WINC_MQTTBrokerSet(wdrvHandle,mqtt_cfg->url, mqtt_cfg->port,tlsHandle );
            if (WDRV_WINC_STATUS_OK != status)
            {
                break;
            }
            
            status = WDRV_WINC_MQTTClientCfgSet(wdrvHandle,mqtt_cfg->clientid,mqtt_cfg->username, 
                    mqtt_cfg->password );
            if (WDRV_WINC_STATUS_OK != status)
            {
                break;
            }
            break;
        }    
		
		/**<Connect to the MQTT Broker */  
        case SYS_WINCS_MQTT_CONNECT:
        {
            SYS_WINCS_MQTT_CFG_t *mqtt_config = (SYS_WINCS_MQTT_CFG_t *)input; 
            
            status =  WDRV_WINC_MQTTConnect(wdrvHandle, mqtt_config->clean_session,
                    mqtt_config->keep_alive_time, mqtt_config->protoVer, SYS_WINCS_MQTT_ConnCallback, 0);
            
            if (WDRV_WINC_STATUS_OK != status)
            {
                break;
            }
			break;
        }
		

        /**<Trigger Disconnect from MQTT Broker*/        
        case SYS_WINCS_MQTT_DISCONNECT:
	    {	
            status = WDRV_WINC_MQTTDisconnect(wdrvHandle, WDRV_WINC_MQTT_DISCONN_REASON_CODE_NORMAL);
            break;
        }
        /**<Subscribe to Topics */
        case SYS_WINCS_MQTT_SUBS_TOPIC:
        {
            SYS_WINCS_MQTT_FRAME_t *mqtt_frame = (SYS_WINCS_MQTT_FRAME_t *)input;
            status =  WDRV_WINC_MQTTSubscribe(wdrvHandle, mqtt_frame->qos,
                   mqtt_frame->topic ,SYS_WINCS_MQTT_SubscribeCallback, 0);
            if (WDRV_WINC_STATUS_OK != status)
            {
                break;
            }
            break;
        }
	
        case SYS_WINCS_MQTT_UNSUBSCRIBE:
        {
            char *topicName = (char *)input;
            status =  WDRV_WINC_MQTTUnsubscribe(wdrvHandle, topicName);
            break;
        }
        /**<Configure the MQTT Application Callback*/ 
        case SYS_WINCS_MQTT_SET_CALLBACK:
        {
            g_MqttCallBackHandler[1] = (SYS_WINCS_MQTT_CALLBACK_t)(input);
            break;
        }

        /**<Configure the MQTT Application Callback*/     
        case SYS_WINCS_MQTT_SET_SRVC_CALLBACK:        
        {
            g_MqttCallBackHandler[0] = (SYS_WINCS_MQTT_CALLBACK_t)(input);   
            break;
        }
        
        
        case SYS_WINCS_MQTT_GET_CALLBACK:
        {
            SYS_WINCS_MQTT_CALLBACK_t *mqttCallBackHandler;
            mqttCallBackHandler = (SYS_WINCS_MQTT_CALLBACK_t *)input;
            
            mqttCallBackHandler[0] = g_MqttCallBackHandler[0];
            mqttCallBackHandler[1] = g_MqttCallBackHandler[1];
            break;
        }
            
        default:
            break;    
    }
    
    return SYS_WINCS_WIFI_GetWincsStatus(status);
}


/* *****************************************************************************
 End of File
 */
