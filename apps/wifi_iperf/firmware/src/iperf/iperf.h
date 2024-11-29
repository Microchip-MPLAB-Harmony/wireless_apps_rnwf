/*
Copyright (C) 2023, Microchip Technology Inc., and its subsidiaries. All rights reserved.

The software and documentation is provided by microchip and its contributors
"as is" and any express, implied or statutory warranties, including, but not
limited to, the implied warranties of merchantability, fitness for a particular
purpose and non-infringement of third party intellectual property rights are
disclaimed to the fullest extent permitted by law. In no event shall microchip
or its contributors be liable for any direct, indirect, incidental, special,
exemplary, or consequential damages (including, but not limited to, procurement
of substitute goods or services; loss of use, data, or profits; or business
interruption) however caused and on any theory of liability, whether in contract,
strict liability, or tort (including negligence or otherwise) arising in any way
out of the use of the software and documentation, even if advised of the
possibility of such damage.

Except as expressly permitted hereunder and subject to the applicable license terms
for any third-party software incorporated in the software and any applicable open
source software license terms, no license or other rights, whether express or
implied, are granted under any patent or other intellectual property rights of
Microchip or any third party.
*/

#ifndef IPERF_H_
#define IPERF_H_

#include "wdrv_winc_client_api.h"

#define TEST_BUFFER_SIZE            1472


typedef union
{
    struct
    {
        sa_family_t         sin_family;
        in_port_t           sin_port;
    };
    struct sockaddr_in      v4;
    struct sockaddr_in6     v6;
} APP_SOCK_ADDR_TYPE;

// used to reference the 4 byte ID number we place in UDP datagrams
// Support 64 bit seqno on machines that support them
typedef struct UDP_datagram
{
    uint32_t id;
    uint32_t tv_sec;
    uint32_t tv_usec;
    uint32_t id2;
} UDP_datagram;

/*
 * The client_hdr structure is sent from clients
 * to servers to alert them of things that need
 * to happen. Order must be perserved in all
 * future releases for backward compatibility.
 * 1.7 has flags, numThreads, mPort, and bufferlen
 */
typedef struct client_hdr {

    /*
     * flags is a bitmap for different options
     * the most significant bits are for determining
     * which information is available. So 1.7 uses
     * 0x80000000 and the next time information is added
     * the 1.7 bit will be set and 0x40000000 will be
     * set signifying additional information. If no
     * information bits are set then the header is ignored.
     * The lowest order diferentiates between dualtest and
     * tradeoff modes, wheither the speaker needs to start
     * immediately or after the audience finishes.
     */
    int32_t flags;
    int32_t numThreads;
    int32_t mPort;
    int32_t bufferlen;
    int32_t mWinBand;
    int32_t mAmount;
} client_hdr;

/*
 * The server_hdr structure facilitates the server
 * report of jitter and loss on the client side.
 * It piggy_backs on the existing clear to close
 * packet.
 */
typedef struct server_hdr {
    /*
     * flags is a bitmap for different options
     * the most significant bits are for determining
     * which information is available. So 1.7 uses
     * 0x80000000 and the next time information is added
     * the 1.7 bit will be set and 0x40000000 will be
     * set signifying additional information. If no
     * information bits are set then the header is ignored.
     */
    int32_t flags;
    int32_t total_len1;
    int32_t total_len2;
    int32_t stop_sec;
    int32_t stop_usec;
    int32_t error_cnt;
    int32_t outorder_cnt;
    int32_t datagrams;
    int32_t jitter1;
    int32_t jitter2;

} server_hdr;

typedef enum
{
    MODE_TCP_CLIENT,
    MODE_TCP_SERVER,
    MODE_UDP_CLIENT,
    MODE_UDP_SERVER
} IPERF_OP_MODE;

typedef struct
{
    IPERF_OP_MODE               opMode;
    uint32_t                    packetToSend;
    uint32_t                    dataRateBps;
    uint32_t                    packetLength;
    APP_SOCK_ADDR_TYPE    remoteIPAddress;
    APP_SOCK_ADDR_TYPE    localIPAddress;
    bool                        useTls;
} IPERF_INIT_DATA;

uint8_t IPERF_GetNumActiveSockets(void);
void IPERF_Init(void);
bool IPERF_Create(IPERF_INIT_DATA* pInit, bool isPaused);
bool IPERF_Start(void);
bool IPERF_Stop(int sockfd);
void IPERF_Update(void);
bool IPERF_setStreamTos(int sockfd, uint8_t tid);

#endif /* IPERF_H_ */
