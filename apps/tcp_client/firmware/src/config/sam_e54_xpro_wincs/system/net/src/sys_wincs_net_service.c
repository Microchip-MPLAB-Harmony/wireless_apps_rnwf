/*******************************************************************************
  WINCS Host Assisted Net Service Implementation

  File Name:
    sys_wincs_net_service.c

  Summary:
    Source code for the WINCS Host Assisted Net Service implementation.

  Description:
    This file contains the source code for the WINCS Host Assisted Net Service
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


/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* This section lists the other files that are included in this file.
 */
#include "system/net/sys_wincs_net_service.h"
#include "system/wifi/sys_wincs_wifi_service.h"


/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */
static SYS_WINCS_NET_SOCK_CALLBACK_t g_SocketCallBackHandler[SYS_WINCS_NET_SOCK_SERVICE_CB_MAX];

static SYS_WINCS_NET_DHCP_CALLBACK_t  g_DHCPCallBackHandler;

static SYS_WINCS_NET_CID_TYPE g_connIDs[SYS_WINCS_NET_NUM_CONN_IDS];

static WDRV_WINC_TLS_HANDLE g_tlsHandle = WDRV_WINC_TLS_INVALID_HANDLE;


/* ************************************************************************** */
/* ************************************************************************** */
// Section: Local Functions                                                   */
/* ************************************************************************** */
/* ************************************************************************** */

static SYS_WINCS_NET_CID_TYPE* SYS_WINCS_NET_ConnIDAdd
(
    int sockfd, 
    SYS_WINCS_NET_SOCKET_TYPE socketType, 
    SYS_WINCS_NET_SOCK_ADDR_TYPE *pLocalBindAddr, 
    SYS_WINCS_NET_SOCK_ADDR_TYPE *pRemoteBindAddr, 
    bool httpChkSum
)
{
    uint8_t i;

    for (i=0; i<SYS_WINCS_NET_NUM_CONN_IDS; i++)
    {
        if (false == g_connIDs[i].inUse)
        {
            memset(&g_connIDs[i], 0, sizeof(SYS_WINCS_NET_CID_TYPE));

            g_connIDs[i].inUse            = true;
            g_connIDs[i].id               = i+1;
            g_connIDs[i].sockfd           = sockfd;
            g_connIDs[i].socketType       = socketType;
            g_connIDs[i].doHTTPChecksum   = httpChkSum;

            if (NULL != pLocalBindAddr)
            {
                memcpy(&g_connIDs[i].localBindAddress, pLocalBindAddr, sizeof(SYS_WINCS_NET_SOCK_ADDR_TYPE));
            }

            if (NULL != pRemoteBindAddr)
            {
                memcpy(&g_connIDs[i].remoteBindAddress, pRemoteBindAddr, sizeof(SYS_WINCS_NET_SOCK_ADDR_TYPE));
            }
            #ifdef SYS_WINCS_NET_DEBUG_LOGS
            SYS_WINCS_NET_DBG_MSG("Adding CID %d\r\n", g_connIDs[i].id);
            #endif
            return &g_connIDs[i];
        }
    }

    return NULL;
}

static SYS_WINCS_NET_CID_TYPE* SYS_WINCS_NET_ConnIDFindBySocket
(
    int sockfd
)
{
    uint8_t i;

    for (i=0; i<SYS_WINCS_NET_NUM_CONN_IDS; i++)
    {
        if ((true == g_connIDs[i].inUse) && (g_connIDs[i].sockfd == sockfd))
        {
            return &g_connIDs[i];
        }
    }

    return NULL;
}


static bool SYS_WINCS_NET_ConnIDDeleteBySocket
(
    int sockfd
)
{
    uint8_t i;

    for (i=0; i<SYS_WINCS_NET_NUM_CONN_IDS; i++)
    {
        if ((true == g_connIDs[i].inUse) && (g_connIDs[i].sockfd == sockfd))
        {
            memset(&g_connIDs[i], 0, sizeof(SYS_WINCS_NET_CID_TYPE));
            return true;
        }
    }
    return false;
}

#ifdef SYS_WINCS_NET_DEBUG_LOGS
static void SYS_WINCS_NET_connIDPrint
(
    void
)
{
    uint8_t i;
    uint8_t n = 0;

    SYS_WINCS_NET_DBG_MSG("Available CIDs:\r\n");

    for (i=0; i<SYS_WINCS_NET_NUM_CONN_IDS; i++)
    {
        if (true == g_connIDs[i].inUse)
        {
            in_port_t port;
            char addrStr[64];

            if (0 != (g_connIDs[i].socketType & SYS_WINCS_NET_SKT_SERVER))
            {
                port = ntohs(g_connIDs[i].localBindAddress.sin_port);
            }
            else
            {
                port = ntohs(g_connIDs[i].remoteBindAddress.sin_port);
            }

            if (AF_INET == g_connIDs[i].remoteBindAddress.sin_family)
            {
                inet_ntop(AF_INET, &g_connIDs[i].remoteBindAddress.v4.sin_addr.s_addr, addrStr, sizeof(addrStr));
            }
            else
            {
                inet_ntop(AF_INET6, &g_connIDs[i].remoteBindAddress.v6.sin6_addr.s6_addr, addrStr, sizeof(addrStr));
            }

            SYS_WINCS_NET_DBG_MSG("CID: %d, sock:%d, port:%u, ip: %s\r\n", g_connIDs[i].id, g_connIDs[i].sockfd, port, addrStr);
            n++;
        }
    }

    if (0 == n)
    {
        SYS_WINCS_NET_DBG_MSG("The list is empty\r\n");
    }
}
#endif


            
static void SYS_WINCS_NET_SocketEventCallback
(
    uintptr_t context, 
    int sockfd, 
    WINC_SOCKET_EVENT event, 
    WINC_SOCKET_STATUS status
)
{
    SYS_WINCS_NET_CID_TYPE *pConnID = SYS_WINCS_NET_ConnIDFindBySocket(sockfd);
    SYS_WINCS_NET_SOCK_CALLBACK_t net_cb_func = g_SocketCallBackHandler[0];
    
    
    #ifdef SYS_WINCS_NET_DEBUG_LOGS
    SYS_WINCS_NET_DBG_MSG("f SYS_WINCS_NET_SocketEventCallback, sockfd : %d event : %d\r\n",sockfd,event);
    #endif
    if(event ==  WINC_SOCKET_EVENT_CLOSE)
    {
        net_cb_func(sockfd, SYS_WINCS_NET_SOCK_EVENT_CLOSED, NULL);
        return;
    }
    
    if (NULL == pConnID)
    {
        SYS_WINCS_NET_DBG_MSG("ERROR : Net Callback NULL");
        return;
    }
    
    if (WINC_SOCKET_STATUS_OK != status)
    {
        SYS_WINCS_NET_DBG_MSG("ERROR: Socket error %d from event %d\r\n", status, event);
        SYS_WINCS_NET_ConnIDDeleteBySocket(sockfd);
        shutdown(sockfd, SHUT_RDWR);
        return;
    }

    switch (event)
    {
        case WINC_SOCKET_EVENT_OPEN:
        {
            if (PF_UNSPEC == pConnID->remoteBindAddress.sin_family)
            {
                if (-1 == bind(sockfd, (struct sockaddr*)&pConnID->localBindAddress, sizeof(SYS_WINCS_NET_SOCK_ADDR_TYPE)))
                {
                    SYS_WINCS_NET_DBG_MSG("ERROR: Failed to bind socket\r\n");
                    SYS_WINCS_NET_ConnIDDeleteBySocket(sockfd);
                    shutdown(sockfd, SHUT_RDWR);
                    break;
                }

                if (SYS_WINCS_NET_SKT_STREAM == (pConnID->socketType & SYS_WINCS_NET_SKT_STREAM))
                {
                    listen(sockfd, 0);
                }

                if (SYS_WINCS_NET_SKT_DATAGRAM == (pConnID->socketType & SYS_WINCS_NET_SKT_DATAGRAM))
                {
                    pConnID->isConnected = true;
                    #ifdef SYS_WINCS_NET_DEBUG_LOGS
                    SYS_WINCS_NET_DBG_MSG("Socket %d session ID = %d\r\n", sockfd, 0);
                    #endif
                }
            }
            else
            {
                if(SYS_WINCS_NET_SKT_MULTICAST == (pConnID->socketType & SYS_WINCS_NET_SKT_MULTICAST))
                {
                    if (AF_INET == pConnID->remoteBindAddress.sin_family)
                    {
                        pConnID->localBindAddress.v4.sin_addr.s_addr = htonl(INADDR_ANY);
                    }
                    else
                    {
                        memcpy(&pConnID->localBindAddress.v6.sin6_addr, &in6addr_any, sizeof(in6addr_any));
                    }

                    pConnID->localBindAddress.sin_family = pConnID->remoteBindAddress.sin_family;
                    pConnID->localBindAddress.sin_port   = pConnID->remoteBindAddress.sin_port;

                    if (-1 == bind(sockfd, (struct sockaddr*)&pConnID->localBindAddress, sizeof(SYS_WINCS_NET_SOCK_ADDR_TYPE)))
                    {
                        SYS_WINCS_NET_DBG_MSG("ERROR: Failed to bind socket\r\n");
                        SYS_WINCS_NET_ConnIDDeleteBySocket(sockfd);
                        shutdown(sockfd, SHUT_RDWR);
                        break;
                    }

                    if (AF_INET == pConnID->remoteBindAddress.sin_family)
                    {
                        struct ip_mreqn group;
                        memcpy(&group.imr_multiaddr, &pConnID->remoteBindAddress.v4.sin_addr, sizeof(struct in_addr));
                        memcpy(&group.imr_address, &pConnID->localBindAddress.v4.sin_addr, sizeof(struct in_addr));

                        group.imr_ifindex = 0;
                        setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &group, sizeof(group));
                    }
                    else
                    {
                        struct ipv6_mreq group;
                        memcpy(&group.ipv6mr_multiaddr, &pConnID->remoteBindAddress.v6.sin6_addr, sizeof(struct in6_addr));
                        group.ipv6mr_interface = 0;
                        setsockopt(sockfd, IPPROTO_IP, IPV6_ADD_MEMBERSHIP, &group, sizeof(group));
                    }
                    pConnID->isConnected = true;
                }
                else
                {
                    if(SYS_WINCS_NET_SKT_ENCRYPTED == (pConnID->socketType & SYS_WINCS_NET_SKT_ENCRYPTED))
                    {
                        setsockopt(sockfd, IPPROTO_TLS, TLS_CONF_IDX, &g_tlsHandle, sizeof(g_tlsHandle));
                    }
                    errno = 0;

                    if (-1 == connect(sockfd, (struct sockaddr*)&pConnID->remoteBindAddress, sizeof(SYS_WINCS_NET_SOCK_ADDR_TYPE)))
                    {
                        if (EINPROGRESS != errno)
                        {
                            SYS_WINCS_NET_DBG_MSG("ERROR: Failed to connect to the server!\r\n");
                            SYS_WINCS_NET_ConnIDDeleteBySocket(sockfd);
                            shutdown(sockfd, SHUT_RDWR);
                            break;
                        }
                    }
                }
            }

            break;
        }

        case WINC_SOCKET_EVENT_LISTEN:
        {
            if(SYS_WINCS_NET_SKT_MULTICAST != (pConnID->socketType & SYS_WINCS_NET_SKT_MULTICAST))
            {
                #ifdef SYS_WINCS_NET_DEBUG_LOGS
                SYS_WINCS_NET_DBG_MSG("Socket %d session ID = %d\r\n", sockfd, 0);
                #endif
                
                #ifdef SYS_WINCS_NET_DEBUG_LOGS
                SYS_WINCS_NET_connIDPrint();
                #endif
                SYS_WINCS_NET_DBG_MSG("Socket Listening......\r\n");
            }
            break;
        }

        case WINC_SOCKET_EVENT_CONNECT_REQ:
        {
            #ifdef SYS_WINCS_NET_DEBUG_LOGS
            SYS_WINCS_NET_DBG_MSG("WINC_SOCKET_EVENT_CONNECT_REQ\r\n");
            #endif
            int newSocket;
            SYS_WINCS_NET_SOCK_ADDR_TYPE addr;
            socklen_t addrLen = sizeof(SYS_WINCS_NET_SOCK_ADDR_TYPE);

            errno = 0;
            newSocket = accept(sockfd, (struct sockaddr*)&addr, &addrLen);

            if (-1 == newSocket)
            {
                if ((EWOULDBLOCK == errno) || (EAGAIN == errno))
                {
                    break;
                }

                SYS_WINCS_NET_DBG_MSG("Accept error!\r\n");
                shutdown(sockfd, SHUT_RDWR);
                break;
            }

            pConnID = SYS_WINCS_NET_ConnIDAdd(newSocket, pConnID->socketType, &pConnID->localBindAddress, &addr, false);

            if (NULL != pConnID)
            {
                #ifdef SYS_WINCS_NET_DEBUG_LOGS
                SYS_WINCS_NET_connIDPrint();
                #endif
                pConnID->isConnected = true;
            }
            net_cb_func(sockfd, SYS_WINCS_NET_SOCK_EVENT_CLIENT_CONNECTED, NULL);
            break;
        }

        case WINC_SOCKET_EVENT_CONNECT:
        {
            #ifdef SYS_WINCS_NET_DEBUG_LOGS
            SYS_WINCS_NET_DBG_MSG("WINC_SOCKET_EVENT_CONNECT\r\n");
            #endif
            if(SYS_WINCS_NET_SKT_ENCRYPTED != (pConnID->socketType & SYS_WINCS_NET_SKT_ENCRYPTED))
            {
                pConnID->isConnected = true;

                #ifdef SYS_WINCS_NET_DEBUG_LOGS
                SYS_WINCS_NET_connIDPrint();
                #endif
                net_cb_func(sockfd, SYS_WINCS_NET_SOCK_EVENT_CONNECTED, NULL);
            }
            break;
        }

        case WINC_SOCKET_EVENT_TLS_CONNECT:
        {
            SYS_WINCS_NET_DBG_MSG("WINC_SOCKET_EVENT_TLS_CONNECT\r\n");
            pConnID->isConnected = true;
            #ifdef SYS_WINCS_NET_DEBUG_LOGS
            SYS_WINCS_NET_connIDPrint();
            #endif
            net_cb_func(sockfd, SYS_WINCS_NET_SOCK_EVENT_TLS_DONE, NULL);
            break;
        }
        
        case WINC_SOCKET_EVENT_SEND:
        {
            #ifdef SYS_WINCS_NET_DEBUG_LOGS
            SYS_WINCS_NET_DBG_MSG("Socket %d send completed\r\n", sockfd);
            #endif
            
            break;
        }
        
        case WINC_SOCKET_EVENT_RECV:
        {
            #ifdef SYS_WINCS_NET_DEBUG_LOGS
            SYS_WINCS_NET_DBG_MSG("WINC_SOCKET_EVENT_RECV from soc : %d\r\n",sockfd);
            #endif
            
            net_cb_func(sockfd, SYS_WINCS_NET_SOCK_EVENT_READ, NULL);
            break;
        }

        case WINC_SOCKET_EVENT_ERROR:
        {
            net_cb_func(sockfd, SYS_WINCS_NET_SOCK_EVENT_ERROR, NULL);
            break;
        }
        

        default:
        {
            break;
        }
    }
}




static void SYS_WINCS_NET_ApDhcpsEventCallback
(
    DRV_HANDLE handle, 
    WDRV_WINC_DHCPS_EVENT_TYPE eventType, 
    void *pEventInfo
)
{
    SYS_WINCS_NET_DHCP_CALLBACK_t net_dhcp_func = g_DHCPCallBackHandler;
    switch (eventType)
    {
        case WDRV_WINC_DHCPS_EVENT_LEASE_ASSIGNED:
        {
            char s[20] = {0};

            WDRV_WINC_UtilsIPAddressToString((WDRV_WINC_IPV4_ADDR*)pEventInfo, s, sizeof(s));
            net_dhcp_func(SYS_WINCS_NET_STA_DHCP_DONE,(uint8_t *) s);
            break;
        }

        default:
        {
            break;
        }
    }
}


SYS_WINCS_RESULT_t SYS_WINCS_NET_CreateSocket
(
    SYS_WINCS_NET_SOCKET_t *new_socket
)
{
    int domain = AF_UNSPEC,socket_id=-1;
    bool httpChkSum = false;
    SYS_WINCS_NET_SOCK_ADDR_TYPE addr;
    SYS_WINCS_NET_SOCKET_TYPE socketType = SYS_WINCS_NET_SKT_UNENCRYPTED;
    WDRV_WINC_IP_MULTI_ADDRESS IPAddress;
    SYS_WINCS_RESULT_t result = SYS_WINCS_FAIL;
           
    /* socket type : TLS or Non TLS*/
    if(new_socket->tls_conf)
    {
        socketType &= ~SYS_WINCS_NET_SKT_UNENCRYPTED;
        socketType |= SYS_WINCS_NET_SKT_ENCRYPTED;
    }
    else
    {
        socketType = SYS_WINCS_NET_SKT_UNENCRYPTED;
    }
    
    /* SOcket type : TCP or UDP */
    if(new_socket->sock_type == SYS_WINCS_SOCK_TCP)
    {
        socketType |= SYS_WINCS_NET_SKT_STREAM;
    }
    else if (new_socket->sock_type == SYS_WINCS_SOCK_UDP)
    {
        socketType |= SYS_WINCS_NET_SKT_DATAGRAM;
    }
    
    /* SOcket type :Client or Server  */
    if(new_socket->bind_type == SYS_WINCS_BIND_REMOTE) //client
    {
        socketType |= SYS_WINCS_NET_SKT_CLIENT;
    }
    else if (new_socket->bind_type == SYS_WINCS_BIND_LOCAL)//server
    {
        socketType |= SYS_WINCS_NET_SKT_SERVER;
    }
    
    /* Addr type : IPV4 or IPV6*/
    if(new_socket->ip_type == SYS_WINCS_NET_IPV4)
    {
        domain = AF_INET;
        if(new_socket->sock_addr != NULL)
        {
            if (1 != inet_pton(AF_INET, new_socket->sock_addr, &IPAddress.v4))
            {
                return result;
            }
        }
    }
    else
    {
        domain = AF_INET6;
        if(1 != inet_pton(AF_INET6, new_socket->sock_addr, &IPAddress.v6))
        {
            return result;
        }
    }
    /* Create TCP or UDP socket*/
    if(new_socket->sock_type == SYS_WINCS_SOCK_TCP)
    {
        if(new_socket->tls_conf)
        {
            socket_id = socket(domain, SOCK_STREAM, IPPROTO_TLS);
        }
        else
        {
            socket_id = socket(domain, SOCK_STREAM, IPPROTO_TCP);
        }
    }
    else if(new_socket->sock_type == SYS_WINCS_SOCK_UDP)
    {
        socket_id = socket(domain, SOCK_DGRAM, IPPROTO_UDP);
    }
    
    if (socket_id < 0)
    {
        return SYS_WINCS_FAIL;
    }
    if (AF_INET == domain)
    {
        if (0 == IPAddress.v4.Val)
        {
            addr.v4.sin_family      = domain;
            addr.v4.sin_port        = htons(new_socket->sock_port);
            addr.v4.sin_addr.s_addr = htonl(INADDR_ANY);
        }
        else
        {
            addr.v4.sin_family      = domain;
            addr.v4.sin_port        = htons(new_socket->sock_port);
            addr.v4.sin_addr.s_addr = IPAddress.v4.Val;
        }
    }
    else
    {
        if (0 == memcmp(&IPAddress.v6, &in6addr_any, sizeof(in6addr_any)))
        {
            addr.v6.sin6_family = AF_INET6;
            addr.v6.sin6_port   = htons(new_socket->sock_port);
            memcpy(&addr.v6.sin6_addr, &in6addr_any, sizeof(in6addr_any));
        }
        else
        {
            addr.v6.sin6_family = AF_INET6;
            addr.v6.sin6_port   = htons(new_socket->sock_port);
            memcpy(&addr.v6.sin6_addr, &IPAddress.v6, sizeof(IPAddress.v6));
        }
    }
    
    if (NULL == SYS_WINCS_NET_ConnIDAdd(socket_id, socketType, NULL, &addr, httpChkSum))
    {
        shutdown(socket_id, SHUT_RDWR);
        return SYS_WINCS_FAIL;
    }
    
    return SYS_WINCS_PASS;
}




/* This function is used for Network and Socket service Control*/
SYS_WINCS_RESULT_t SYS_WINCS_NET_SockSrvCtrl
( 
    SYS_WINCS_NET_SOCK_SERVICE_t request, 
    void *input
)
{
    DRV_HANDLE wdrvHandle = DRV_HANDLE_INVALID;
    
    SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_GET_DRV_HANDLE,&wdrvHandle);
    WDRV_WINC_STATUS status = WDRV_WINC_STATUS_OK;
    
    switch(request)
    {
        /**<Enable the DHCP server */
        case SYS_WINCS_NET_DHCP_SERVER_ENABLE:
        {
            if(input == NULL)
                break;
            
            WDRV_WINC_IP_MULTI_ADDRESS apIPv4Addr;
            WDRV_WINC_IPV4_ADDR dhcpsIPv4PoolAddr;
            
            const char **dhcps_cfg_list = input; 
            
            if (false == WDRV_WINC_UtilsStringToIPAddress(dhcps_cfg_list[0], &apIPv4Addr.v4))
            {
                return SYS_WINCS_FAIL;
            }
            
            if (false == WDRV_WINC_UtilsStringToIPAddress(dhcps_cfg_list[1], &dhcpsIPv4PoolAddr))
            {
                return SYS_WINCS_FAIL;
            }

            status = WDRV_WINC_NetIfIPAddrSet(wdrvHandle, WDRV_WINC_NETIF_IDX_0, WDRV_WINC_IP_ADDRESS_TYPE_IPV4, &apIPv4Addr, 24);
            if(status != WDRV_WINC_STATUS_OK)
            {
                break;
            }

            status = WDRV_WINC_DHCPSPoolAddressSet(wdrvHandle, WDRV_WINC_DHCPS_IDX_0, &dhcpsIPv4PoolAddr);
            if(status != WDRV_WINC_STATUS_OK)
            {
                break;
            } 

            status = WDRV_WINC_DHCPSEnableSet(wdrvHandle, WDRV_WINC_DHCPS_IDX_0, true, SYS_WINCS_NET_ApDhcpsEventCallback);
            if(status != WDRV_WINC_STATUS_OK)
            {
                break;
            }
            
            break;
        }
	
        /**<Disable the DHCP server */ 
        case SYS_WINCS_NET_DHCP_SERVER_DISABLE:
        {
            status =  WDRV_WINC_DHCPSEnableSet(wdrvHandle, WDRV_WINC_DHCPS_IDX_0, false, NULL);
            break;
        }
	
        /**<Open TCP Socket*/ 
        case SYS_WINCS_NET_SOCK_TCP_OPEN: 
        {  
            WDRV_WINC_SocketRegisterEventCallback(wdrvHandle, &SYS_WINCS_NET_SocketEventCallback);
            SYS_WINCS_NET_CreateSocket((SYS_WINCS_NET_SOCKET_t*)(input)); 
            
            break;   
            SYS_WINCS_NET_SOCKET_t *new_socket=(SYS_WINCS_NET_SOCKET_t*)(input); 
            
            switch(new_socket->bind_type)
            {
                case SYS_WINCS_BIND_REMOTE:
                {
                    SYS_WINCS_NET_CreateSocket(new_socket);
                    break;
                }
                
                case SYS_WINCS_BIND_LOCAL:
                {
                    if(new_socket->sock_type == SYS_WINCS_SOCK_UDP)
                    {
                        SYS_WINCS_NET_CreateSocket(new_socket);
                        break;
                    }
                    int domain,socket_id;
                    bool httpChkSum;
                    SYS_WINCS_NET_SOCK_ADDR_TYPE addr;
                    SYS_WINCS_NET_SOCKET_TYPE socketType;
                    
                    if(new_socket->tls_conf)
                    {
                        socketType = SYS_WINCS_NET_SKT_ENCRYPTED;
                    }
                    else
                    {
                        socketType = SYS_WINCS_NET_SKT_UNENCRYPTED;
                    }
                    
                    socketType |= (SYS_WINCS_NET_SKT_STREAM | SYS_WINCS_NET_SKT_SERVER);
                    domain = AF_INET;
                    WDRV_WINC_SocketRegisterEventCallback(wdrvHandle, &SYS_WINCS_NET_SocketEventCallback);
                    
                    socket_id = socket(domain, SOCK_STREAM,  IPPROTO_TCP);
                    
                    if(new_socket->ip_type == SYS_WINCS_NET_IPV4)
                    {
                        addr.v4.sin_family      = domain;
                        addr.v4.sin_port        = htons(new_socket->sock_port);
                        addr.v4.sin_addr.s_addr = htonl(INADDR_ANY);
                    }
                    else
                    {
                        addr.v6.sin6_family = AF_INET6;
                        addr.v6.sin6_port   = htons(new_socket->sock_port);
                        memcpy(&addr.v6.sin6_addr, &in6addr_any, sizeof(in6addr_any));
                    }
            
                    httpChkSum = false;
                    #if SYS_RNWF_NET_DEBUG_PRINT
                    SYS_CONSOLE_PRINT("socketType : %d\r\n",socketType);
                    #endif
                    

                    if (NULL == SYS_WINCS_NET_ConnIDAdd(socket_id, socketType, &addr, NULL, httpChkSum))
                    {
                        SYS_CONSOLE_PRINT("ERROR: Failed to add CID\r\n");
                        shutdown(socket_id, SHUT_RDWR);
                        return SYS_WINCS_FAIL;
                    }
                    #ifdef SYS_RNWF_NET_DEBUG_PRINT
                    SYS_RNWF_NET_ConnIDPrint();
                    #endif
                    break;
                }
                
                case SYS_WINCS_BIND_MCAST:
                {
                    break;
                }
                
                case SYS_WINCS_BIND_NONE:
                {
                    break;
                }
            }
            break;
        }
            
        /**<Close the socket*/    
        case SYS_WINCS_NET_SOCK_CLOSE:
        {
            uint32_t socket = *((uint32_t *)input);
            SYS_WINCS_NET_ConnIDDeleteBySocket(socket);
            
            if(0 == shutdown(socket, SHUT_RDWR))
            {
                status = WDRV_WINC_STATUS_OK;
            }
            break;
        }
	
        case SYS_WINCS_NET_OPEN_TLS_CTX:
        {
            g_tlsHandle = WDRV_WINC_TLSCtxOpen(wdrvHandle);
            *(WDRV_WINC_TLS_HANDLE *)input = g_tlsHandle;
            break;
        }
        
        case SYS_WINCS_NET_GET_TLS_CTX_HANDLE:
        {
            *(WDRV_WINC_TLS_HANDLE *)input = g_tlsHandle;
            break;
        }
        
        /**<Use the TLS configurations */
        case SYS_WINCS_NET_TLS_CONFIG_1:
        case SYS_WINCS_NET_TLS_CONFIG_2:
        {
            const char **tls_cfg_list = input;
            if(tls_cfg_list[SYS_WINCS_NET_TLS_CA_CERT] != NULL)
            {
                status = WDRV_WINC_TLSCtxCACertFileSet(wdrvHandle, g_tlsHandle, tls_cfg_list[SYS_WINCS_NET_TLS_CA_CERT] , false);
                if (WDRV_WINC_STATUS_OK != status)
                {
                    break;
                }
            }
            
            if(tls_cfg_list[SYS_WINCS_NET_TLS_CERT_NAME] != NULL)
            {
                status =  WDRV_WINC_TLSCtxCertFileSet(wdrvHandle, g_tlsHandle, tls_cfg_list[SYS_WINCS_NET_TLS_CERT_NAME]);
                if (WDRV_WINC_STATUS_OK != status)
                {
                    break;
                }
            }
            
            if((tls_cfg_list[SYS_WINCS_NET_TLS_KEY_NAME] != NULL) && (tls_cfg_list[SYS_WINCS_NET_TLS_KEY_PWD] != NULL))
            {
                status =  WDRV_WINC_TLSCtxPrivKeySet(wdrvHandle, g_tlsHandle, tls_cfg_list[SYS_WINCS_NET_TLS_KEY_NAME], tls_cfg_list[SYS_WINCS_NET_TLS_KEY_PWD]);
                if (WDRV_WINC_STATUS_OK != status)
                {
                    break;
                }
            }
            
            if(tls_cfg_list[SYS_WINCS_NET_TLS_SERVER_NAME] != NULL)
            {
                status =  WDRV_WINC_TLSCtxSNISet(wdrvHandle, g_tlsHandle, tls_cfg_list[SYS_WINCS_NET_TLS_SERVER_NAME]);
                if (WDRV_WINC_STATUS_OK != status)
                {
                    break;
                }
            }
            
//            if(tls_cfg_list[SYS_WINCS_NET_TLS_DOMAIN_NAME] != NULL)
            {
                status =  WDRV_WINC_TLSCtxHostnameCheckSet(wdrvHandle, g_tlsHandle, tls_cfg_list[SYS_WINCS_NET_TLS_DOMAIN_NAME], false);
                if (WDRV_WINC_STATUS_OK != status)
                {
                    break;
                }
            }
     
            break;
        }

        /**<Register application callback for sockets*/
        case SYS_WINCS_NET_SOCK_SET_CALLBACK:
	    {
            g_SocketCallBackHandler[0] = (SYS_WINCS_NET_SOCK_CALLBACK_t)(input);                        
            break;
	    }    
        
        /**<Register application callback for sockets*/
        case SYS_WINCS_NET_SOCK_SET_SRVC_CALLBACK:
	    {
            g_DHCPCallBackHandler  = (SYS_WINCS_NET_DHCP_CALLBACK_t)(input);  
            break;
	    }   
	    
        /*<Get Function callback data*/        
        case SYS_WINCS_NET_SOCK_GET_CALLBACK:
        {
            SYS_WINCS_NET_SOCK_CALLBACK_t *socketCallBackHandler;
            socketCallBackHandler = (SYS_WINCS_NET_SOCK_CALLBACK_t *) input;
            socketCallBackHandler[0] = g_SocketCallBackHandler[0];
            break;
        }
        
        default:
	    {
            break;
        }
    } 

    return SYS_WINCS_WIFI_GetWincsStatus(status);
}



/*This function is used to write into TCP Socket*/
SYS_WINCS_RESULT_t SYS_WINCS_NET_TcpSockWrite
( 
    uint32_t socket, 
    uint16_t length, 
    uint8_t *input
)
{
    SYS_WINCS_RESULT_t result = SYS_WINCS_PASS;

    int16_t sent_data_len = 0;
    errno = 0;
    
    sent_data_len = send(socket, input, length, 0);
    if (-1 == sent_data_len)
    {
        SYS_WINCS_NET_DBG_MSG("ERROR: Failed to send to socket %d (%d)\r\n", socket, errno);
        result = SYS_WINCS_FAIL;
    }
    return result;
}


/*This function is used to read from the socket*/
int16_t SYS_WINCS_NET_SockRead
(
    uint32_t socket, 
    uint16_t length, 
    uint8_t *buffer
) 
{                
    int16_t recvd_data_len = 0;
    recvd_data_len = recv(socket, buffer, length, 0);

    if (-1 == recvd_data_len)
    {
        if ((EWOULDBLOCK != errno) && (EAGAIN != errno))
        {
            SYS_WINCS_NET_DBG_MSG("Socket recv error %d %d\r\n",socket, errno);
            SYS_WINCS_NET_ConnIDDeleteBySocket(socket);
        }
    }
    return recvd_data_len;
}


/*This function is used to read from the TCP socket*/
int16_t SYS_WINCS_NET_TcpSockRead
( 
    uint32_t socket, 
    uint16_t  length, 
    uint8_t *buffer
)  
{
    return SYS_WINCS_NET_SockRead(socket, length, buffer);
}


/*This function is used to read from the UDP socket*/
int16_t SYS_WINCS_NET_UdpSockRead
( 
    uint32_t socket, 
    uint16_t length, 
    uint8_t *buffer
)  
{                   
    return SYS_WINCS_NET_SockRead(socket, length, buffer);
}



/*This function is used to write into UDP Socket*/
SYS_WINCS_RESULT_t SYS_WINCS_NET_UdpSockWrite
(
    uint32_t socket, 
    const char *addr, 
    uint32_t port, 
    uint16_t length, 
    uint8_t *input
)
{
    SYS_WINCS_RESULT_t result = SYS_WINCS_FAIL;
    WDRV_WINC_IP_MULTI_ADDRESS remoteIPAddress;
    SYS_WINCS_NET_SOCK_ADDR_TYPE ip_addr;
        
    if (1 == inet_pton(AF_INET, (const char *)addr, &remoteIPAddress.v4))
    {
        ip_addr.v4.sin_family      = AF_INET;
        ip_addr.v4.sin_port        = htons(port);
        ip_addr.v4.sin_addr.s_addr = remoteIPAddress.v4.Val;
    }
    else if (1 == inet_pton(AF_INET6,(const char *) addr, &remoteIPAddress.v6))
    {
        ip_addr.v6.sin6_family = AF_INET6;
        ip_addr.v6.sin6_port   = htons(port);
        memcpy(&ip_addr.v6.sin6_addr, &remoteIPAddress.v6, sizeof(remoteIPAddress.v6));
    }
    else
    {
        return result;
    }
    
    if (-1 == sendto(socket, input, length, 0, (struct sockaddr*)&ip_addr, sizeof(ip_addr)))
    {
        SYS_WINCS_NET_DBG_MSG("ERROR: Failed to send to socket %d (%d)\r\n", socket, errno);
        return result;
    }
    return SYS_WINCS_PASS;
}

/* *****************************************************************************
 End of File
 */
