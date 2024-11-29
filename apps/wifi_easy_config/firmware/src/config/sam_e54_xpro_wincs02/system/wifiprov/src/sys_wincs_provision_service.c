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
    .bindType = SYS_WINCS_NET_BIND_TYPE0,   
    
    /*Socket port number */
    .sockPort = SYS_WINCS_NET_SOCK_PORT0,
    
    /*Socket type(TCP/UDP)*/
    .sockType = SYS_WINCS_NET_SOCK_TYPE0,

    .ipType  =  SYS_WINCS_NET_SOCK_TYPE_IPv4_0,

    .tlsEnable = SYS_WINCS_TLS_ENABLE0,
};


/* ************************************************************************** */
/* ************************************************************************** */
// Section: Local Functions                                                   */
/* ************************************************************************** */
/* ************************************************************************** */


/* ************************************************************************** */
/**
 * @brief Parses the security type and returns the corresponding WiFi security type.
 *
 * This function takes a security type as input and maps it to the corresponding
 * `SYS_WINCS_WIFI_SECURITY_t` enumeration value. If the security type is invalid,
 * it defaults to `SYS_WINCS_WIFI_SECURITY_OPEN` and logs an error message.
 *
 * @param secType The security type to be parsed. This is a uint8_t value.
 *
 * @return The corresponding `SYS_WINCS_WIFI_SECURITY_t` value.
 */

static SYS_WINCS_WIFI_SECURITY_t SYS_WINCS_PROV_ParseAuth
(
    uint8_t secType
)
{
    // Initialize the authentication type to open security by default
    SYS_WINCS_WIFI_SECURITY_t authType = SYS_WINCS_WIFI_SECURITY_OPEN;

    // Map the input security type to the corresponding authentication type
    switch(secType)
    {
        case 1:
            // Map security type 1 to the corresponding auth type
            authType = secType - 1;
            break;

        case 2:
            // Map security type 2 to the corresponding auth type
            authType = secType;
            break;

        case 4:
            // Map security type 4 to the corresponding auth type
            authType = secType + 1;
            break;

        default:
            // Log an error message for invalid security types
            SYS_WINCS_PROV_DBG_MSG("Invalid security type\r\n");
            break;
    }

    // Return the determined authentication type
    return authType;
}

         
/* ************************************************************************** */
/**
 * @brief Parses the WiFi configuration string and populates the WiFi configuration structure.
 *
 * This function takes a WiFi configuration string and parses it to populate the
 * `SYS_WINCS_WIFI_PARAM_t` structure with the SSID, security type, and passphrase.
 * It also sets the channel and auto-connect parameters.
 *
 * @param wifiCofnig The WiFi configuration string to be parsed. This is a uint8_t pointer.
 * @param wifi_config The WiFi configuration structure to be populated. This is a pointer
 *                    to `SYS_WINCS_WIFI_PARAM_t`.
 *
 * @return The result of the parsing operation. This is of type `SYS_WINCS_RESULT_t`.
 */

static SYS_WINCS_RESULT_t SYS_WINCS_PROV_AppParse
(
    uint8_t *wifiCofnig,
    SYS_WINCS_WIFI_PARAM_t *wifi_config
)
{
    char* p;    
    SYS_WINCS_RESULT_t ret = SYS_WINCS_PASS;

    // Tokenize the input string using comma as the delimiter
    p = strtok((char *)wifiCofnig, ",");
    if (p != NULL && !strncmp(p, SYS_WINCS_APP_WIFI_PROV_CONFIG_ID, strlen(SYS_WINCS_APP_WIFI_PROV_CONFIG_ID))) 
    {
        // Parse the SSID
        p = strtok(NULL, ",");
        if (p)
            wifi_config->ssid = p;

        // Parse the security type
        p = strtok(NULL, ",");
        if (p) 
        {
            uint8_t security = (SYS_WINCS_WIFI_SECURITY_t)atoi(p);

            // Parse the authentication type based on the security type
            wifi_config->security = SYS_WINCS_PROV_ParseAuth(security);

            if (SYS_WINCS_WIFI_SECURITY_OPEN < wifi_config->security && wifi_config->security <= SYS_WINCS_WIFI_SECURITY_WPA3)
            {
                // Parse the passphrase if the security type requires it
                p = strtok(NULL, ",");
                if (p) 
                    wifi_config->passphrase = p;
                else
                    ret = SYS_WINCS_FAIL;
            } 
            else if (wifi_config->security == SYS_WINCS_WIFI_SECURITY_OPEN)
                wifi_config->passphrase = NULL;
            else
                ret = SYS_WINCS_FAIL;
        }
        else
            ret = SYS_WINCS_FAIL;

        // Set the channel and auto-connect parameters
        wifi_config->channel = WDRV_WINC_CID_ANY;
        wifi_config->autoConnect = true;

        // Print debug message with the parsed configuration
        SYS_CONSOLE_PRINT("\r\n");
        SYS_WINCS_PROV_DBG_MSG("Connecting to SoftAP : SSID:%s - PASSPHRASE:%s - AUTH:%d\r\n", 
                            wifi_config->ssid, 
                            wifi_config->passphrase, 
                            wifi_config->security
                            );
    }
    else if (p != NULL && !strncmp(p, SYS_WINCS_APP_WIFI_PROV_DONE_ID, strlen(SYS_WINCS_APP_WIFI_PROV_DONE_ID)))
    {
        // Handle the case where the provisioning is done
    }
    return ret;
}


/* ************************************************************************** */
/**
 * @brief Processes the WiFi provisioning data received over a socket.
 *
 * This function reads the provisioning data from the specified socket, parses it,
 * and if successful, closes the socket and triggers the provisioning callback with
 * the parsed WiFi configuration.
 *
 * @param socket The socket from which to read the provisioning data. This is a uint32_t value.
 *
 * @return The result of the processing operation. This is of type `SYS_WINCS_RESULT_t`.
 */

static SYS_WINCS_RESULT_t SYS_WINCS_PROV_AppProcess
(
    uint32_t socket
) 
{
    // Structure to hold the parsed WiFi configuration
    SYS_WINCS_WIFI_PARAM_t wifiConfig;

    // Buffer to hold the received provisioning data
    uint8_t prov_buf[SYS_WINCS_PROV_BUF_LEN_MAX];

    // Read data from the socket into the buffer
    if (SYS_WINCS_NET_TcpSockRead(socket, SYS_WINCS_PROV_RECV_BUFFER_SIZE, (uint8_t *)prov_buf) > 0)
    {
        // Parse the received provisioning data
        if (SYS_WINCS_PROV_AppParse(prov_buf, &wifiConfig) == SYS_WINCS_PASS)
        {
            // Close the socket after successful parsing
            if (SYS_WINCS_FAIL == SYS_WINCS_NET_SockSrvCtrl(SYS_WINCS_NET_SOCK_CLOSE, &socket))
            {
                return SYS_WINCS_FAIL;
            }

            // Set additional WiFi configuration parameters
            wifiConfig.mode        = SYS_WINCS_WIFI_MODE_STA;
            wifiConfig.autoConnect = true;
            wifiConfig.channel     = WDRV_WINC_CID_ANY;

            // Trigger the provisioning callback with the parsed WiFi configuration
            if (g_provCallBackHandler)
                g_provCallBackHandler(SYS_WINCS_PROV_COMPLETE, (uint8_t *)&wifiConfig);

            return SYS_WINCS_PASS;
        }
        else
        {
            // Trigger the provisioning failure callback if parsing fails
            if (g_provCallBackHandler)
                g_provCallBackHandler(SYS_WINCS_PROV_FAILURE, NULL);
        }
    }

    // Return failure if reading from the socket or parsing fails
    return SYS_WINCS_FAIL;
}


/* ************************************************************************** */
/**
 * @brief Provisioning socket callback function.
 *
 * This function handles various socket events such as connection, disconnection,
 * data read, and client connection. It performs appropriate actions based on the
 * event type.
 *
 * @param sock The socket associated with the event. This is a uint32_t value.
 * @param event The type of socket event. This is of type `SYS_WINCS_NET_SOCK_EVENT_t`.
 * @param p_str Additional data associated with the event, if any. This is a uint8_t pointer.
 */
static void SYS_WINCS_PROV_SocketCallback(uint32_t sock, SYS_WINCS_NET_SOCK_EVENT_t event, uint8_t *p_str)
{
    switch(event)
    {
        case SYS_WINCS_NET_SOCK_EVENT_CONNECTED:
        {
            // Handle socket connected event
            break;
        }

        case SYS_WINCS_NET_SOCK_EVENT_DISCONNECTED:
        {
            // Handle socket disconnected event
            // Close the socket
            SYS_WINCS_NET_SockSrvCtrl(SYS_WINCS_NET_SOCK_CLOSE, &sock);
            break;
        }

        case SYS_WINCS_NET_SOCK_EVENT_READ:
        {
            // Handle socket read event
            // Process the provisioning data received on the socket
            
            if (SYS_WINCS_FAIL == SYS_WINCS_PROV_AppProcess(sock))
            {
                SYS_WINCS_PROV_DBG_MSG("Failed to parse Socket data \r\n");
            }
            break;
        }

        case SYS_WINCS_NET_SOCK_EVENT_CLIENT_CONNECTED:
        {
            // Handle client connected event
            // Log a message indicating a client has connected
            SYS_WINCS_PROV_DBG_MSG("Client Socket Connected!\r\n");
            break;
        }

        default:
        {
            // Handle any other events
            break;
        }
    }
}


/* ************************************************************************** */
/**
 * @brief WiFi event callback function.
 *
 * This function handles various WiFi events such as connection, disconnection,
 * and other WiFi-related events. It performs appropriate actions based on the
 * event type.
 *
 * @param event The type of WiFi event. This is of type `SYS_WINCS_WIFI_EVENT_t`.
 * @param wifiHandle The handle associated with the WiFi event. This is of type `SYS_WINCS_WIFI_HANDLE_t`.
 */
void SYS_WINCS_PROV_WifiCallback
(
    SYS_WINCS_WIFI_EVENT_t event, 
    SYS_WINCS_WIFI_HANDLE_t wifiHandle
)
{
            
    switch(event)
    {
        /* SNTP UP event code*/
        case SYS_WINCS_WIFI_SNTP_UP:
        {            
            SYS_WINCS_PROV_DBG_MSG("SNTP UP\r\n");  
        }
        break;

        /* Wi-Fi connected event code*/
        case SYS_WINCS_WIFI_CONNECTED:
        {
            SYS_WINCS_PROV_DBG_MSG("Wi-Fi Connected    \r\n");
            break;
        }
        
        /* Wi-Fi disconnected event code*/
        case SYS_WINCS_WIFI_DISCONNECTED:
        {
            SYS_WINCS_PROV_DBG_MSG("Wi-Fi Disconnected\nReconnecting... \r\n");
            break;
        }
        
        /* Wi-Fi DHCP complete event code*/
        case SYS_WINCS_WIFI_DHCP_IPV4_COMPLETE:
        {         
            SYS_WINCS_PROV_DBG_MSG("DHCP IPv4 : %s\r\n", (uint8_t *)wifiHandle);
            break;
        }
        
        case SYS_WINCS_WIFI_DHCP_IPV6_LOCAL_COMPLETE:
        {
            //SYS_WINCS_PROV_DBG_MSG("[APP] : DHCP IPv6 Local : %s\r\n", (uint8_t *)wifiHandle);
            break;
        }
        
        case SYS_WINCS_WIFI_DHCP_IPV6_GLOBAL_COMPLETE:
        {
            //SYS_WINCS_PROV_DBG_MSG("[APP] : DHCP IPv6 Global: %s\r\n", (uint8_t *)wifiHandle);
            break;
        }
        
        default:
        {
            break;
        }
                    
    }    
}


/* ************************************************************************** */
/**
 * @brief Network DHCP event callback function.
 *
 * This function handles various DHCP events such as obtaining an IP address.
 * It performs appropriate actions based on the event type.
 *
 * @param event The type of DHCP event. This is of type `SYS_WINCS_NET_DHCP_EVENT_t`.
 * @param p_str Additional data associated with the event, typically the obtained IP address. This is a uint8_t pointer.
 */

static void SYS_WINCS_PROV_NetCallback
(
    SYS_WINCS_NET_DHCP_EVENT_t event, 
    uint8_t *p_str
)
{
    switch(event)
    {
        case SYS_WINCS_NET_STA_DHCP_DONE:
        {
            // Handle DHCP done event for STA (Station)
            // Log a message indicating the obtained IP address
            SYS_WINCS_PROV_DBG_MSG("STA Connected -> DHCP IP : %s\r\n", p_str);

            // Open a TCP socket for provisioning
            
            if (SYS_WINCS_FAIL == SYS_WINCS_NET_SockSrvCtrl(SYS_WINCS_NET_SOCK_TCP_OPEN, &g_provisionSocket))
            {
                SYS_WINCS_PROV_DBG_MSG("Failed to create socket\r\n");
            }
            break;
        }

        default:
        {
            // Handle any other events (if any)
            break;
        }
    }
    return;
}



/**
 * @brief Provisioning service control function.
 *
 * This function handles various provisioning service control requests such as enabling,
 * disabling, and setting callbacks for the provisioning service. It performs appropriate
 * actions based on the request type.
 *
 * @param request The type of provisioning service request. This is of type `SYS_WINCS_PROV_SERVICE_t`.
 * @param provHandle The handle associated with the provisioning service request. This is of type `SYS_WINCS_PROV_HANDLE_t`.
 *
 * @return The result of the service control operation. This is of type `SYS_WINCS_RESULT_t`.
 */
SYS_WINCS_RESULT_t SYS_WINCS_PROV_SrvCtrl
(
    SYS_WINCS_PROV_SERVICE_t request,
    SYS_WINCS_PROV_HANDLE_t provHandle
)
{
    switch(request)
    {
        case SYS_WINCS_PROV_ENABLE:
        {                   
            // Register application callbacks for network and socket events
            SYS_WINCS_NET_SockSrvCtrl(SYS_WINCS_NET_SOCK_SET_SRVC_CALLBACK, SYS_WINCS_PROV_NetCallback);
            SYS_WINCS_NET_SockSrvCtrl(SYS_WINCS_NET_SOCK_SET_CALLBACK, SYS_WINCS_PROV_SocketCallback);

            // Configure and enable DHCP server
            const char *dhcps_cfg[] = {SYS_WINCS_WIFI_AP_IP_ADDR, SYS_WINCS_WIFI_AP_IP_POOL_START};
            
            if (SYS_WINCS_FAIL == SYS_WINCS_NET_SockSrvCtrl(SYS_WINCS_NET_DHCP_SERVER_ENABLE, dhcps_cfg))
            {
                return SYS_WINCS_FAIL;
            }

            // Configure Wi-Fi Access Point (AP) settings
            SYS_WINCS_WIFI_PARAM_t wifi_ap_cfg = {
                .mode           = SYS_WINCS_WIFI_DEVMODE,
                .ssid           = SYS_WINCS_WIFI_PROV_SSID, 
                .passphrase     = SYS_WINCS_WIFI_PROV_PWD,
                .security       = SYS_WINCS_WIFI_PROV_SECURITY,
                .channel        = SYS_WINCS_WIFI_PROV_CHANNEL,
                .ssidVisibility = SYS_WINCS_WIFI_PROV_SSID_VISIBILITY
            };   

            // Set Wi-Fi parameters and enable AP mode
            if (SYS_WINCS_FAIL == SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_SET_PARAMS, &wifi_ap_cfg))
            {
                return SYS_WINCS_FAIL;
            }
            if (SYS_WINCS_FAIL == SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_AP_ENABLE, NULL))
            {
                return SYS_WINCS_FAIL;
            }

            // Log messages indicating provisioning mode is enabled
            SYS_WINCS_PROV_DBG_MSG("Provision mode enabled\r\n");
            SYS_WINCS_PROV_DBG_MSG("Connect to SSID : %s   ,PASSPHRASE : %s ,AUTH : %d\r\n", 
                                wifi_ap_cfg.ssid, 
                                wifi_ap_cfg.passphrase,
                                wifi_ap_cfg.security);

            break;
        }

        case SYS_WINCS_PROV_DISABLE:
        {
            // Unregister application callbacks and disable AP mode
            SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_SET_SRVC_CALLBACK, NULL);

            if (SYS_WINCS_FAIL == SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_AP_DISABLE, NULL))
            {
                return SYS_WINCS_FAIL;
            }

            // Unregister network callbacks and disable DHCP server
            SYS_WINCS_NET_SockSrvCtrl(SYS_WINCS_NET_SOCK_SET_SRVC_CALLBACK, NULL);
            
            if (SYS_WINCS_FAIL == SYS_WINCS_NET_SockSrvCtrl(SYS_WINCS_NET_DHCP_SERVER_DISABLE, NULL))
            {
                return SYS_WINCS_FAIL;
            }
            break;
        }

        case SYS_WINCS_PROV_SET_CALLBACK:
        {
            // Set the provisioning callback handler
            if(provHandle != NULL)
                g_provCallBackHandler = (SYS_WINCS_PROV_CALLBACK_t)provHandle;
            break;
        }    

        default:
        {
            // Handle any other requests (if any)
            break;
        }
    }

    return SYS_WINCS_PASS;
}

/* *****************************************************************************
 End of File
 */
