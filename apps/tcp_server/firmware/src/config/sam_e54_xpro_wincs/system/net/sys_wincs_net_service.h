/*******************************************************************************
  WINCS Host Assisted Net Service Header file 

  File Name:
    sys_wincs_net_service.h

  Summary:
    Header file for the WINCS Host Assisted Net Service implementation.

  Description:
    This file contains the header file for the WINCS Host Assisted Net Service
    implementation.
 *******************************************************************************/

//DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (C) 2020 released Microchip Technology Inc.  All rights reserved.

 * Microchip Technology Inc. and its subsidiaries.  You may use this software 
 * and any derivatives exclusively with Microchip products. 
 * 
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER 
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED 
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A 
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION 
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS 
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE 
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS 
 * IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF 
 * ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE 
 * TERMS. 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef SYS_WINCS_NET_SERVICE_H
#define	SYS_WINCS_NET_SERVICE_H

#include <xc.h>
#include "wdrv_winc.h"
#include "wdrv_winc_client_api.h"
#include "system/wifi/sys_wincs_wifi_service.h"  


#define SYS_WINCS_NET_DBG_MSG(args, ...)      SYS_CONSOLE_PRINT("[NET]:"args, ##__VA_ARGS__)

//#define SYS_WINCS_NET_DEBUG_PRINT 0
/*WINCS Network Socket max callback service */
#define SYS_WINCS_NET_SOCK_SERVICE_CB_MAX    2

#define SYS_WINCS_SOCK_ID_LEN_MAX            8
#define SYS_WINCS_SOCK_ADDR_LEN_MAX          32
#define SYS_WINCS_SOCK_TLS_CFG_LEN_MAX       64

#define SYS_WINCS_NET_NUM_CONN_IDS        10
#define SYS_WINCS_NET_SOCK_RCV_BUF_SIZE     2048





/**
 @brief Network and Socket service List
 */
typedef enum 
{
    /**<Use the TLS configuration 1*/
    SYS_WINCS_NET_TLS_CONFIG_1 = 1, 
    
    /**<Use the TLS configuration 2*/
    SYS_WINCS_NET_TLS_CONFIG_2,       
            
    /**<Network Interface configuration*/        
    SYS_WINCS_NET_IF_CONFIG,                 
     
    /**<Enable the DHCP server */
    SYS_WINCS_NET_DHCP_SERVER_ENABLE,
     
    /**<Disable the DHCP server */        
    SYS_WINCS_NET_DHCP_SERVER_DISABLE,  
            
    /**<Open TCP Socket*/        
    SYS_WINCS_NET_SOCK_TCP_OPEN,     
    
    /**<Open UDP Socket*/        
    SYS_WINCS_NET_SOCK_UDP_OPEN,     
            
    /**<Close the socket*/        
    SYS_WINCS_NET_SOCK_CLOSE,     
            
    /**<Register application callback for sockets*/
    SYS_WINCS_NET_SOCK_SET_CALLBACK,         
    SYS_WINCS_NET_SOCK_SET_SRVC_CALLBACK,
            
    /*<Get Function callback data*/        
    SYS_WINCS_NET_SOCK_GET_CALLBACK,
    
    /* OPen TLX context*/
    SYS_WINCS_NET_OPEN_TLS_CTX,

    /* Get TLX context*/
    SYS_WINCS_NET_GET_TLS_CTX_HANDLE,

}SYS_WINCS_NET_SOCK_SERVICE_t;



/**
 @brief Socket Type
 
 */
typedef enum   
{    
    /**<UDP Socket type*/
    SYS_WINCS_SOCK_UDP = 1,                  
            
    /**<TCP Socket type*/
    SYS_WINCS_SOCK_TCP,  

}SYS_WINCS_SOCK_TYPE_t;

/**
 @brief Socket Bind Type
 
 */
typedef enum    
{ 
    /**<Bind as a server Socket*/
    SYS_WINCS_BIND_LOCAL,         
            
    /**<Bind as a client Socket*/        
    SYS_WINCS_BIND_REMOTE,          
     
     /**<Bind as a multicast Socket*/
    SYS_WINCS_BIND_MCAST,             
           
    /**Bind None**/        
    SYS_WINCS_BIND_NONE,

}SYS_WINCS_BIND_TYPE_t;

/**
 @brief Socket Type
 
 */
typedef enum    
{ 
    /**<ASCII Socket mode*/
    SYS_WINCS_ASCII_MODE = 1,      
            
    /**<Binary Socket mode*/        
    SYS_WINCS_BINARY_MODE,  

}SYS_WINCS_SOCK_RW_MODE_t;



/**
 @brief Network socket events
 */
typedef enum 
{
    /**<Socket connected event*/
    SYS_WINCS_NET_SOCK_EVENT_CONNECTED,  
            
    /**<TLS handshake done event*/
    SYS_WINCS_NET_SOCK_EVENT_TLS_DONE,
            
    /**<Socket disonnected event*/
    SYS_WINCS_NET_SOCK_EVENT_DISCONNECTED,   
            
    /**<Socket data read event*/        
    SYS_WINCS_NET_SOCK_EVENT_READ,          
        
    /**<Socket error event*/        
    SYS_WINCS_NET_SOCK_EVENT_ERROR,
            
    /**<Socket event undefined*/
    SYS_WINCS_NET_SOCK_EVENT_UNDEFINED,
         
    /* Client Connected */
    SYS_WINCS_NET_SOCK_EVENT_CLIENT_CONNECTED,
            
    SYS_WINCS_NET_SOCK_EVENT_CLOSED,

}SYS_WINCS_NET_SOCK_EVENT_t;


typedef enum 
{
    /* STA ip address received */
    SYS_WINCS_NET_STA_DHCP_DONE,
            
}SYS_WINCS_NET_DHCP_EVENT_t;

/**
 @brief Network socket TLS Configurations
 */
typedef enum 
{
    /* TLS CA Certificate */
    SYS_WINCS_NET_TLS_CA_CERT = 0, 

    /* TLS Certificate Name */         
    SYS_WINCS_NET_TLS_CERT_NAME,

    /* TLS Key name  */            
    SYS_WINCS_NET_TLS_KEY_NAME,

    /* TLS Key password  */
    SYS_WINCS_NET_TLS_KEY_PWD,

    /* TLS Server name  */
    SYS_WINCS_NET_TLS_SERVER_NAME,
            
    /* TLS Domain Name */
    SYS_WINCS_NET_TLS_DOMAIN_NAME,

}SYS_WINCS_NET_TLS_CONFIG_ID_t;


/**
 @brief Network socket IP Configurations
 */
typedef enum 
{
    /**<Open socket with IPv4 address */
        SYS_WINCS_NET_IPV4 = 4,
 
    /**<Open socket with IPv6 locak address */
        SYS_WINCS_NET_IPV6_LOCAL = 6,
    /**<Open socket with IPv6 global address */
        SYS_WINCS_NET_IPV6_GLOBAL = 6,
                
}SYS_WINCS_NET_IP_TYPE_t;


/**
 @brief Socket data type
 
 */
typedef struct 
{    
    /**<Bind type of socket*/
    SYS_WINCS_BIND_TYPE_t    bind_type;  
    
    /**<UDP or TCP socket type*/
    SYS_WINCS_SOCK_TYPE_t    sock_type;        
    
    /**<Server or Client port number*/
    uint16_t            sock_port;          
    
    /**<Socket Address (IPv4 Address)*/
    const char          *sock_addr;         
    
    /**<Server Socket ID*/
    uint32_t            sock_master;       
    
    /**<TLS configuration */
    uint8_t             tls_conf; 
    
    uint8_t             ip_type;

}SYS_WINCS_NET_SOCKET_t;


/**
 @brief Advanced Socket settings
 
 */
typedef struct 
{  
    /**<Socket ID */
    uint32_t    sock_id;                
    
    /**<Keep-Alive option*/
    uint8_t     sock_keepalive;         
    
    /**<Socket NAGLE/NoDelay*/
    uint8_t     sock_nodelay;  

}SYS_WINCS_NET_SOCKET_CONFIG_t;



typedef union
{
    struct
    {
        sa_family_t         sin_family;
        in_port_t           sin_port;
    };
    struct sockaddr_in      v4;
    struct sockaddr_in6     v6;
    
} SYS_WINCS_NET_SOCK_ADDR_TYPE;


typedef enum
{
    SYS_WINCS_NET_SKT_CLIENT        = 0x0001,
    SYS_WINCS_NET_SKT_SERVER        = 0x0002,
    SYS_WINCS_NET_SKT_STREAM        = 0x0004,
    SYS_WINCS_NET_SKT_DATAGRAM      = 0x0008,
    SYS_WINCS_NET_SKT_UNENCRYPTED   = 0x0010,
    SYS_WINCS_NET_SKT_ENCRYPTED     = 0x0020,
    SYS_WINCS_NET_SKT_MULTICAST     = 0x0040,
            
} SYS_WINCS_NET_SOCKET_TYPE;



typedef struct
{
    uint8_t                     id;
    int                         sockfd;
    bool                        inUse;
    bool                        isConnected;
    SYS_WINCS_NET_SOCKET_TYPE    socketType;
    SYS_WINCS_NET_SOCK_ADDR_TYPE localBindAddress;
    SYS_WINCS_NET_SOCK_ADDR_TYPE remoteBindAddress;
    bool                        doHTTPChecksum;
    size_t                      httpBytesReceived;
} SYS_WINCS_NET_CID_TYPE;



typedef SYS_WINCS_RESULT_t (*SYS_WINCS_NET_DHCP_CALLBACK_t)(SYS_WINCS_NET_DHCP_EVENT_t, uint8_t *);

/**
 @brief Network socket events callback function type 
 */


typedef SYS_WINCS_RESULT_t (*SYS_WINCS_NET_SOCK_CALLBACK_t)(uint32_t sock, SYS_WINCS_NET_SOCK_EVENT_t, uint8_t *);

/**
 * @brief NET Sock Service Layer API to handle system operations.
 * 
 *
 * @param[in] request       Requested service ::SYS_WINCS_NET_SOCK_SERVICE_t
 * @param[in] input         Input/Output data for the requested service 
 * 
 * @return ::SYS_WINCS_PASS Requested service is handled successfully
 * @return ::SYS_WINCS_FAIL Requested service has failed
 */
SYS_WINCS_RESULT_t SYS_WINCS_NET_SockSrvCtrl( SYS_WINCS_NET_SOCK_SERVICE_t request, void *input);


/**
 * @brief NET Socket Write API to send data over TCP socket.
 * 
 *
 * @param[in] socket        Socket ID 
 * @param[in] length        Length of data to be read 
 * @param[in] input         Input buffer to read the data     
 * @param[in] mode          Write mode for the socket ::SYS_WINCS_SOCK_RW_MODE_t
 * 
 * @return ::SYS_WINCS_PASS Requested service is handled successfully
 * @return ::SYS_WINCS_FAIL Requested service has failed
 */
SYS_WINCS_RESULT_t SYS_WINCS_NET_TcpSockWrite( uint32_t socket, uint16_t length, uint8_t *input);

/**
 * @brief NET Socket Write API to send data UDP socket.
 * 
 *
 * @param[in] socket        Socket ID 
 * @param[in] addr          IP address of the UDP peer
 * @param[in] port          Port address of the UDP peer
 * @param[in] length        Length of data to be read 
 * @param[in] input         Input buffer to read the data     

 * 
 * @return ::SYS_WINCS_PASS Requested service is handled successfully
 * @return ::SYS_WINCS_FAIL Requested service has failed
 */
SYS_WINCS_RESULT_t SYS_WINCS_NET_UdpSockWrite( uint32_t socket, const char *addr, uint32_t port, uint16_t length, uint8_t *input);

/**
 * @brief NET Socket Read API to read data from TCP Socket.
 * 
 *
 * @param[in] socket        Socket ID 
 * @param[in] length        Length of data to be read 
 * @param[in] input         Input buffer to read the data     
 * 
 * @return ::SYS_WINCS_PASS Requested service is handled successfully
 * @return ::SYS_WINCS_FAIL Requested service has failed
 */
int16_t SYS_WINCS_NET_TcpSockRead( uint32_t socket, uint16_t length, uint8_t *input);


/**
 * @brief NET Socket Read API to read data from UDP socket.
 * 
 *
 * @param[in] socket        Socket ID 
 * @param[in] length        Length of data to be read 
 * @param[in] input         Input buffer to read the data     
 * 
 * @return ::SYS_WINCS_PASS Requested service is handled successfully
 * @return ::SYS_WINCS_FAIL Requested service has failed
 */
int16_t SYS_WINCS_NET_UdpSockRead( uint32_t socket, uint16_t length, uint8_t *input);

#endif	/* SYS_WINCS_NET_SERVICE_H */

/** @}*/
