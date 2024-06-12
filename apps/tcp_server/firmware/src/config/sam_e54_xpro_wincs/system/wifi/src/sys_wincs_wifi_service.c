/*******************************************************************************
  WINCS Host Assisted Wifi Service Implementation

  File Name:
    sys_wincs_wifi_service.c

  Summary:
    Source code for the WINCS Host Assisted wifi Service implementation.

  Description:
    This file contains the source code for the WINCS Host Assisted wifiService
    implementation.
 ******************************************************************************/

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
#include <string.h>
#include <time.h>
/* This section lists the other files that are included in this file.
 */

#include "system/wifi/sys_wincs_wifi_service.h"
#include "driver/wifi/wincs02/include/wdrv_winc.h"
#include "driver/wifi/wincs02/include/wdrv_winc_client_api.h"
/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */
SYS_WINCS_WIFI_CALLBACK_t g_wifiCallBackHandler[SYS_WINCS_WIFI_SERVICE_CB_MAX];

static WDRV_WINC_AUTH_CONTEXT authContext;

static WDRV_WINC_BSS_CONTEXT  bssContext;

static uint8_t bssFindIdx;

static OSAL_SEM_HANDLE_TYPE bssFindResult;

static DRV_HANDLE wdrvHandle = DRV_HANDLE_INVALID;

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Local Functions                                                   */
/* ************************************************************************** */
/* ************************************************************************** */

/* AP callback */
static void SYS_WINCS_WIFI_ApNotifyCallback
(
    DRV_HANDLE handle, 
    WDRV_WINC_ASSOC_HANDLE assocHandle, 
    WDRV_WINC_CONN_STATE currentState
)
{
    
    switch (currentState)
    {
        case WDRV_WINC_CONN_STATE_CONNECTED:
        {
            WDRV_WINC_STATUS status;
            WDRV_WINC_MAC_ADDR macAddr;
            
            status = WDRV_WINC_AssocPeerAddressGet(assocHandle, &macAddr);
            if (WDRV_WINC_STATUS_OK == status)
            {
                #if SYS_WINCS_WIFI_DEBUG_LOGS
                SYS_WINCS_WIFI_DBG_MSG("Wifi State :: CONNECTED :: MAC : %02X:%02X:%02X:%02X:%02X:%02X\r\n",
                                        macAddr.addr[0], macAddr.addr[1], macAddr.addr[2],
                                        macAddr.addr[3], macAddr.addr[4], macAddr.addr[5]);
                #endif
            }
            
            break;
        }
        
        case WDRV_WINC_CONN_STATE_CONNECTING:
        {
            SYS_WINCS_WIFI_DBG_MSG("Wifi State :: Connecting.....\r\n");
            break;
        }
        
        case WDRV_WINC_CONN_STATE_FAILED:
        {
            SYS_WINCS_WIFI_DBG_MSG("Wifi State :: Failed \r\n");
            break;
        }
        
        case WDRV_WINC_CONN_STATE_DISCONNECTED:
        {
            SYS_WINCS_WIFI_DBG_MSG("Wifi State DISCONNECTED \r\n");
            break;
        }
        
        case WDRV_WINC_CONN_STATE_ROAMED:
        {
            break;
        }
    }
}




static bool SYS_WINCS_WIFI_BssFindNotifyCallback
(
    DRV_HANDLE handle, 
    uint8_t index, 
    uint8_t ofTotal, 
    WDRV_WINC_BSS_INFO *pBSSInfo
)
{
    
    if (0 == ofTotal)
    {
        SYS_WINCS_WIFI_DBG_MSG("No AP Found Rescan\r\n");
    }
    else
    {
        if (1 == index)
        {
            SYS_WINCS_WIFI_DBG_MSG("#%02d\r\n", ofTotal);
            SYS_WINCS_WIFI_DBG_MSG(">>#  RI  Recommend CH BSSID             SSID\r\n");
            SYS_WINCS_WIFI_DBG_MSG(">>#      Auth Type\r\n>>#\r\n");
        }

        bssFindIdx = index;

        OSAL_SEM_Post(&bssFindResult);
    }

    return false;
}


static void SYS_WINCS_WIFI_ConnectNotifyCallback 
( 
    DRV_HANDLE handle, 
    WDRV_WINC_ASSOC_HANDLE assocHandle, 
    WDRV_WINC_CONN_STATE currentState
)
{
    SYS_WINCS_WIFI_CALLBACK_t wifi_cb_func = g_wifiCallBackHandler[1];
    
    #if SYS_WINCS_WIFI_DEBUG_LOGS
    SYS_WINCS_WIFI_DBG_MSG("SYS_WINCS_WIFI_ConnectNotifyCallback : currentState %d\r\n",currentState);
    #endif   
    switch(currentState)
    {
        /* Association state is disconnected. */
        case WDRV_WINC_CONN_STATE_DISCONNECTED:
        {
            wifi_cb_func(SYS_WINCS_DISCONNECTED, NULL);
            break;
        }

        /* Association state is connecting. */
        case WDRV_WINC_CONN_STATE_CONNECTING:
        {
            break;
        }

        /* Association state is connected. */
        case WDRV_WINC_CONN_STATE_CONNECTED:
        {
            wifi_cb_func(SYS_WINCS_CONNECTED, NULL);
            break;
        }

        /* Association state is connection failed. */
        case WDRV_WINC_CONN_STATE_FAILED:
        {
            wifi_cb_func(SYS_WINCS_WIFI_FAILED, NULL);
            break;
        }

        /* Association state is roamed. */
        case WDRV_WINC_CONN_STATE_ROAMED:
        {
            break;
        }
    }
}


static void SYS_WINCS_WIFI_DhcpEventCallback
(
    DRV_HANDLE handle, 
    WDRV_WINC_NETIF_IDX ifIdx, 
    WDRV_WINC_NETIF_EVENT_TYPE eventType, 
    void *pEventInfo
)
{
    SYS_WINCS_WIFI_CALLBACK_t wifi_cb_func = g_wifiCallBackHandler[1];
    
    switch (eventType)
    {
        case WDRV_WINC_NETIF_EVENT_ADDR_UPDATE:
        {
            WDRV_WINC_NETIF_ADDR_UPDATE_TYPE *pAddrUpdateInfo = pEventInfo;

            if (NULL != pAddrUpdateInfo)
            {
                if (WDRV_WINC_IP_ADDRESS_TYPE_IPV4 == pAddrUpdateInfo->type)
                {
                    char s[20];
                    WDRV_WINC_UtilsIPAddressToString(&pAddrUpdateInfo->addr.v4, s, sizeof(s));
                    wifi_cb_func(SYS_WINCS_DHCP_DONE,(uint8_t*) s);
                }
                else if (WDRV_WINC_IP_ADDRESS_TYPE_IPV6 == pAddrUpdateInfo->type)
                {
                    char s[64];
                    WDRV_WINC_UtilsIPv6AddressToString(&pAddrUpdateInfo->addr.v6, s, sizeof(s));
                    if(strstr(s,"fe80"))
                    {
                        wifi_cb_func(SYS_WINCS_DHCP_IPV6_LOCAL_DONE,(uint8_t*) s);
                    }
                    else
                    {
                        wifi_cb_func(SYS_WINCS_DHCP_IPV6_GLOBAL_DONE,(uint8_t*) s);
                    }
                }
            }
            break;
        }

        case WDRV_WINC_NETIF_EVENT_INTF_UP:
        {
            SYS_WINCS_WIFI_DBG_MSG("WDRV_WINC_NETIF_EVENT_INTF_UP \r\n" );
            break;
        }
        
        default:
        {
            break;
        }
    }
}



static void SYS_WINCS_WIFI_ResolveCallback
(
    DRV_HANDLE handle, 
    WDRV_WINC_DNS_STATUS_TYPE status, 
    uint8_t recordType, 
    const char *pDomainName, 
    WDRV_WINC_IP_MULTI_TYPE_ADDRESS *pIPAddr
)
{
    
    char addrStr[64];
    if ((WDRV_WINC_DNS_STATUS_OK != status) || (NULL == pIPAddr))
    {
        SYS_WINCS_WIFI_DBG_MSG("DNS resolve failed (%d)\r\n", status);
        return;
    }

    if (pIPAddr->type == WDRV_WINC_IP_ADDRESS_TYPE_IPV4)
    {
        inet_ntop(AF_INET, &pIPAddr->addr, addrStr, sizeof(addrStr));
    }
    else if (pIPAddr->type == WDRV_WINC_IP_ADDRESS_TYPE_IPV6)
    {
        inet_ntop(AF_INET6, &pIPAddr->addr, addrStr, sizeof(addrStr));
    }
    else
    {
        SYS_WINCS_WIFI_DBG_MSG("DNS resolve type error (%d)\r\n", pIPAddr->type);
    }

    SYS_WINCS_WIFI_DBG_MSG("Domain Name: '%s' : (%d) -> %s\n", pDomainName, pIPAddr->type, addrStr);
    SYS_WINCS_WIFI_CALLBACK_t wifi_cb_func = g_wifiCallBackHandler[1];
    wifi_cb_func(SYS_WINCS_DNS_RESOLVED, (void *)addrStr);

}


static void SYS_WINCS_SYSTEM_TimeGetCurrentCallback
(
    DRV_HANDLE handle, 
    uint32_t timeUTC
)
{
    struct tm *ptm;
    
    ptm = gmtime(&timeUTC);
    static bool print = true;

    if(print == true)
    {
        if ((ptm->tm_year+1900) > 2000)
        {
            SYS_WINCS_WIFI_DBG_MSG("Time UTC : %d\r\n",timeUTC);
            SYS_WINCS_WIFI_DBG_MSG("Time: %02d:%02d:%02d of %02d/%02d/%02d\r\n",
                    ptm->tm_hour, ptm->tm_min, ptm->tm_sec,
                    ptm->tm_mday, ptm->tm_mon+1, ptm->tm_year+1900);
            
            
            SYS_WINCS_WIFI_CALLBACK_t wifi_cb_func = g_wifiCallBackHandler[1];
            wifi_cb_func(SYS_WINCS_SNTP_UP, (void *)&timeUTC);
            
            print = false;
        }
    }
}



static void SYS_WINCS_WIFI_getRegDomainCallback
(
    DRV_HANDLE handle, 
    uint8_t index, 
    uint8_t ofTotal, 
    bool isCurrent, 
    const WDRV_WINC_REGDOMAIN_INFO *const pRegDomInfo
)
{
    if (0 == ofTotal)
    {
        if ((NULL == pRegDomInfo) || (0 == pRegDomInfo->regDomainLen))
        {
            SYS_WINCS_WIFI_DBG_MSG("No Regulatory Domains Defined\r\n");
        }
        else
        {
            SYS_WINCS_WIFI_DBG_MSG( "WIFI ERROR : GET_REG_DOMAIN\r\n");
        }
    }
    else if (NULL != pRegDomInfo)
    {
        if (1 == index)
        {
            SYS_WINCS_WIFI_DBG_MSG("#.   CC      Channels Ver Status\r\n");
        }
        SYS_WINCS_WIFI_DBG_MSG("%02d: [%-6s] 0x%0-6x %d.%d %s\r\n", index, pRegDomInfo->regDomain, 0, 0, 0, isCurrent ? "Active" : "");
        if (index == ofTotal)
        {
            SYS_WINCS_WIFI_DBG_MSG("DONE \r\n");
        }
    }
    else
    {
        SYS_WINCS_WIFI_DBG_MSG( "WIFI ERROR : GET_REG_DOMAIN");
    }
}


static void SYS_WINCS_WIFI_setRegDomainCallback
(
    DRV_HANDLE handle, 
    uint8_t index, 
    uint8_t ofTotal, 
    bool isCurrent, 
    const WDRV_WINC_REGDOMAIN_INFO *const pRegDomInfo
)
{
    if ((1 != index) || (1 != ofTotal) || (false == isCurrent) || (NULL == pRegDomInfo) || (0 == pRegDomInfo->regDomainLen))
    {
        SYS_WINCS_WIFI_DBG_MSG("WIFI ERROR : SET_REG_DOMAIN");
    }
    else
    {
        SYS_WINCS_WIFI_DBG_MSG("WIFI : SET_REG_DOMAIN SUCCESS");
    }
}

/* Get WINCS Status */
SYS_WINCS_RESULT_t SYS_WINCS_WIFI_GetWincsStatus
(
    WDRV_WINC_STATUS status
)
{
    if( WDRV_WINC_STATUS_OK == status)
    {
        return SYS_WINCS_PASS;
    }
    else if((WDRV_WINC_STATUS_RETRY_REQUEST == status) || (WDRV_WINC_STATUS_BUSY == status))
    {
        return SYS_WINCS_BUSY;
    }
    else
    {
        SYS_WINCS_WIFI_DBG_MSG("[ERROR] : Error in WINCS Service \r\n");
        return SYS_WINCS_FAIL;
    }
}

/* Set Authentication type*/
static WDRV_WINC_STATUS SYS_WINCS_WIFI_SetAuthPersonalType
(
    SYS_WINCS_WIFI_PARAM_t *wifi_config,
    WDRV_WINC_AUTH_TYPE authType
)
{
    return WDRV_WINC_AuthCtxSetPersonal(&authContext, (uint8_t*)wifi_config ->passphrase 
                            , strlen(wifi_config ->passphrase), authType);
}


/* Set Wifi Params */
static WDRV_WINC_STATUS SYS_WINCS_WIFI_SetWifiParams
(
    SYS_WINCS_WIFI_PARAM_t *wifi_config
)
{
    WDRV_WINC_STATUS status = WDRV_WINC_STATUS_NOT_CONNECTED;
    uint8_t ssidLength;
    WDRV_WINC_CHANNEL_ID channel;

    ssidLength = strlen(wifi_config -> ssid);
    channel = wifi_config->channel;

    status = WDRV_WINC_BSSCtxSetDefaults(&bssContext);
    if(status != WDRV_WINC_STATUS_OK)
    {
        return status;
    }

    status = WDRV_WINC_AuthCtxSetDefaults(&authContext);
    if(status != WDRV_WINC_STATUS_OK)
    {
        return status;
    }

    status =  WDRV_WINC_BSSCtxSetSSID(&bssContext, (uint8_t*)wifi_config -> ssid, ssidLength);
    if(status != WDRV_WINC_STATUS_OK)
    {
        return status;
    }
                
    status =  WDRV_WINC_BSSCtxSetChannel(&bssContext, channel);
    if(status != WDRV_WINC_STATUS_OK)
    {
        return status;
    }
                
    /* Security Configuration*/
    switch (wifi_config -> security)
    {
        case SYS_WINCS_OPEN:
        {
            /* Initialize the authentication context for open mode. */
            status =  WDRV_WINC_AuthCtxSetOpen(&authContext);
            break;
        }
        case SYS_WINCS_WPA2:
        {
            status =  SYS_WINCS_WIFI_SetAuthPersonalType(wifi_config, WDRV_WINC_AUTH_TYPE_WPA2_PERSONAL);
            break;
        }

        case SYS_WINCS_WPA2_MIXED:
        {
            status =  SYS_WINCS_WIFI_SetAuthPersonalType(wifi_config, WDRV_WINC_AUTH_TYPE_WPAWPA2_PERSONAL);
            break;
        }

        case SYS_WINCS_WPA2WPA3_PERSONAL:
        {
            status =  SYS_WINCS_WIFI_SetAuthPersonalType(wifi_config, WDRV_WINC_AUTH_TYPE_WPA2WPA3_PERSONAL);

            break;
        }

        case SYS_WINCS_WPA3:
        {
            status =  SYS_WINCS_WIFI_SetAuthPersonalType(wifi_config, WDRV_WINC_AUTH_TYPE_WPA3_PERSONAL);
            break;
        }

        default:
        {
            SYS_WINCS_WIFI_DBG_MSG("[ERROR_WIFI] : INVALID_PARAMETER for AUTHENTICATION\r\n");
            return status;
        }
    }

    if(wifi_config->mode == SYS_WINCS_WIFI_MODE_STA)
    {
        if(wifi_config->autoconnect)
        {
            WDRV_WINC_NetIfRegisterEventCallback(wdrvHandle, SYS_WINCS_WIFI_DhcpEventCallback);
            #ifdef SYS_WINCS_WIFI_DEBUG_LOGS_PRINT
            SYS_WINCS_WIFI_DBG_MSG("[WIFI] : Connecting to AP : %s\r\n",wifi_config->ssid);
            #endif
            status = WDRV_WINC_BSSConnect(wdrvHandle, &bssContext, &authContext,SYS_WINCS_WIFI_ConnectNotifyCallback);
        }
    }
    else if(wifi_config->mode == SYS_WINCS_WIFI_MODE_AP)                
    {        
        status =  WDRV_WINC_BSSCtxSetSSIDVisibility(&bssContext, wifi_config ->ssidVisibility);
    }
    
    return status;
}

/*Wi-Fi Service Control Function*/
SYS_WINCS_RESULT_t SYS_WINCS_WIFI_SrvCtrl
( 
    SYS_WINCS_WIFI_SERVICE_t request, 
    void *input
)
{
    WDRV_WINC_STATUS status = WDRV_WINC_STATUS_OK;
    
    switch (request)
    {
        case SYS_WINCS_WIFI_GET_DRV_STATUS:
        {
            *(int8_t*)input = '\0';
            *(int8_t*)input = WDRV_WINC_Status(sysObj.drvWifiWinc);
            break;
        }
        
        case SYS_WINCS_WIFI_GET_DRV_HANDLE:
        {
            *(DRV_HANDLE *)input = wdrvHandle;
            break;
        }
        
        case SYS_WINCS_WIFI_OPEN_DRIVER:
        {
            if (DRV_HANDLE_INVALID == wdrvHandle)
            {
                wdrvHandle = WDRV_WINC_Open(0, 0);
                if (DRV_HANDLE_INVALID == wdrvHandle)
                {
                    SYS_WINCS_WIFI_DBG_MSG("ERROR : DRV_HANDLE_INVALID\r\n");
                    status = WDRV_WINC_STATUS_NOT_OPEN;
                    break;
                }
            }
            *(DRV_HANDLE *)input = wdrvHandle;
            break;
        }
        
        /* WINCS Get Time */
        case SYS_WINCS_WIFI_GET_TIME:
        {
            status = WDRV_WINC_SystemTimeGetCurrent(wdrvHandle, SYS_WINCS_SYSTEM_TimeGetCurrentCallback);
            break;
        }
        
        /* WINCS Set SNTP */
        case SYS_WINCS_WIFI_SET_SNTP:
        {            
            status =  WDRV_WINC_SNTPServerAddressSet(wdrvHandle, (char *)input);
            if (WDRV_WINC_STATUS_OK != status )
            {
                break;
            }
            status =   WDRV_WINC_SNTPStaticSet(wdrvHandle, true);
            if (WDRV_WINC_STATUS_OK != status )
            {
                break;
            }
            status =  WDRV_WINC_SNTPEnableSet(wdrvHandle, true);
            break;
        }
        /**<Request/Trigger Wi-Fi connect */
        case SYS_WINCS_WIFI_STA_CONNECT:
        {
            status = WDRV_WINC_NetIfRegisterEventCallback(wdrvHandle, SYS_WINCS_WIFI_DhcpEventCallback);
            if(status != WDRV_WINC_STATUS_OK)
            {
                break;
            }
            status = WDRV_WINC_BSSConnect(wdrvHandle, &bssContext, &authContext,SYS_WINCS_WIFI_ConnectNotifyCallback);
            break;
        }

         /**<Request/Trigger Wi-Fi disconnect */    
        case SYS_WINCS_WIFI_STA_DISCONNECT:
        {
            status = WDRV_WINC_BSSDisconnect(wdrvHandle);
            break;
        }

        /**<Request/Trigger SoftAP disable */       
        case SYS_WINCS_WIFI_AP_DISABLE:
        {
            status = WDRV_WINC_APStop(wdrvHandle);
            if(status != WDRV_WINC_STATUS_OK)
            {
                break;
            }

            status =  WDRV_WINC_DHCPSEnableSet(wdrvHandle, WDRV_WINC_DHCPS_IDX_0, false, NULL);
            break;
        }

         /**<Configure the Wi-Fi channel */    
        case SYS_WINCS_WIFI_SET_CHANNEL_AP:
        {      
            uint8_t channel = *(uint8_t *)input;
            status =  WDRV_WINC_BSSCtxSetChannel(&bssContext, channel);
            break;            
        }   
        
        /**<Configure the Access point's BSSID */ 
        case SYS_WINCS_WIFI_SET_BSSID:
        {  
            uint8_t *newBssid = (uint8_t *)input;
            WDRV_WINC_BSSCtxSetBSSID(&bssContext, newBssid);
            break;
        } 
        
        /**<Configure Hidden mode SSID in SoftAP mode*/     
        case SYS_WINCS_WIFI_SET_HIDDEN:
        {        
            status =  WDRV_WINC_BSSCtxSetSSIDVisibility(&bssContext, false);
            break;
        }  

        /**<Configure the Wi-Fi parameters */ 
        case SYS_WINCS_WIFI_SET_PARAMS:  
        {
             status = SYS_WINCS_WIFI_SetWifiParams((SYS_WINCS_WIFI_PARAM_t *)input);
            break;            
        }
        
        case SYS_WINCS_WIFI_AP_ENABLE:
        {
            status = WDRV_WINC_APStart(wdrvHandle, &bssContext, &authContext
                    , SYS_WINCS_WIFI_ApNotifyCallback);
            break;
        }

        /**<Request/Trigger Wi-Fi passive scan */ 
        case SYS_WINCS_WIFI_PASSIVE_SCAN:
        {
            SYS_WIFI_WINC_SCAN_PARAM_t *scanParams = (SYS_WIFI_WINC_SCAN_PARAM_t *)input;
            
            status = WDRV_WINC_BSSFindSetScanParameters(wdrvHandle, 0, 0, scanParams->scanTime, 0);
            if(status != WDRV_WINC_STATUS_OK)
            {
                break;
            }

            status =  WDRV_WINC_BSSFindFirst(wdrvHandle, scanParams->channel, false, NULL, SYS_WINCS_WIFI_BssFindNotifyCallback);
            break;
        }

        /**<Request/Trigger Wi-Fi active scan */    
        case SYS_WINCS_WIFI_ACTIVE_SCAN:
        {
            SYS_WIFI_WINC_SCAN_PARAM_t *scanParams = (SYS_WIFI_WINC_SCAN_PARAM_t *)input;
            
            OSAL_SEM_Create(&bssFindResult, OSAL_SEM_TYPE_COUNTING, 255, 0);
            status =  WDRV_WINC_BSSFindFirst(wdrvHandle, scanParams->channel, true, NULL, SYS_WINCS_WIFI_BssFindNotifyCallback);
            break;
        }
        
        case SYS_WINCS_WIFI_SET_REG_DOMAIN:
        {
            uint8_t *regDomainName = (uint8_t *)input;
            status = WDRV_WINC_WifiRegDomainSet(wdrvHandle,regDomainName, strlen((const char *)regDomainName),SYS_WINCS_WIFI_setRegDomainCallback );
            break;
        }
        
        case SYS_WINCS_WIFI_GET_REG_DOMAIN:
        {
            status = WDRV_WINC_WifiRegDomainGet(wdrvHandle, WDRV_WINC_REGDOMAIN_SELECT_ALL, SYS_WINCS_WIFI_getRegDomainCallback );
            break;
        }
        

        /**<Regester the call back for async events */    
        case SYS_WINCS_WIFI_SET_CALLBACK:  
        {
            g_wifiCallBackHandler[1] = (SYS_WINCS_WIFI_CALLBACK_t)input;
            break;
        }

        /**<Regester the call back for async events */
        case SYS_WINCS_WIFI_SET_SRVC_CALLBACK:                        
        {
            g_wifiCallBackHandler[0] = (SYS_WINCS_WIFI_CALLBACK_t)input;  
            break;
        }
        
        case SYS_WINCS_WIFI_DNS_RESOLVE:
        {
            char *domainName = (char *)input;
            uint16_t timeoutMs = 100;
            status = WDRV_WINC_DNSResolveDomain(wdrvHandle, WINC_CONST_DNS_TYPE_A,
                    domainName, timeoutMs, SYS_WINCS_WIFI_ResolveCallback);
            break;
        }
        
        case SYS_WINCS_WIFI_GET_CALLBACK:
        {
            SYS_WINCS_WIFI_CALLBACK_t *callBackHandler;
            callBackHandler = (SYS_WINCS_WIFI_CALLBACK_t *)input;
            
            callBackHandler[0] = g_wifiCallBackHandler[0];
            callBackHandler[1] = g_wifiCallBackHandler[1];
            break;
        }
        
        default:
        {
            status = WDRV_WINC_STATUS_OPERATION_NOT_SUPPORTED;
            break;
        }
    }  
    
    
    return SYS_WINCS_WIFI_GetWincsStatus(status);
}


/* *****************************************************************************
 End of File
 */
