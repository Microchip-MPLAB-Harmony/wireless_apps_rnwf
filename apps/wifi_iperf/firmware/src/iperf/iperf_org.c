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

#include <stdbool.h>
#include <stdint.h>

#include "iperf/iperf.h"

#define IPERF_TCP_TX_STATE_UNKNOWN                  0
#define IPERF_TCP_TX_STATE_OPENING                  1
#define IPERF_TCP_TX_STATE_CONNECT                  2
#define IPERF_TCP_TX_STATE_CONNECTING               3
#define IPERF_TCP_TX_STATE_CONNECTED                4
#define IPERF_TCP_TX_STATE_RUNNING                  5
#define IPERF_TCP_TX_STATE_STOP                     6

#define IPERF_TCP_RX_STATE_UNKNOWN                  10
#define IPERF_TCP_RX_STATE_OPENING                  11
#define IPERF_TCP_RX_STATE_BIND                     12
#define IPERF_TCP_RX_STATE_RUNNING_LISTENER         13
#define IPERF_TCP_RX_STATE_RUNNING_CLIENT           14
#define IPERF_TCP_RX_STATE_STOP                     15

#define IPERF_UDP_TX_STATE_UNKNOWN                  20
#define IPERF_UDP_TX_STATE_OPENING                  21
#define IPERF_UDP_TX_STATE_CONNECT                  22
#define IPERF_UDP_TX_STATE_HOLD                     23
#define IPERF_UDP_TX_STATE_RUNNING                  24
#define IPERF_UDP_TX_STATE_SEND_CLOSE_REPORT        25
#define IPERF_UDP_TX_STATE_WAIT_FOR_SERVER_REPORT   26
#define IPERF_UDP_TX_STATE_STOP                     27

#define IPERF_UDP_RX_STATE_UNKNOWN                  30
#define IPERF_UDP_RX_STATE_OPENING                  31
#define IPERF_UDP_RX_STATE_BIND                     32
#define IPERF_UDP_RX_STATE_RUNNING                  33
#define IPERF_UDP_RX_STATE_STOP                     35

typedef struct
{
    uint32_t high;
    uint32_t low;
} IPERF_UINT64;

#define IPERF_CONNECTION_TIMEOUT   (10 * 1000)
#define IPERF_TX_TIMEOUT           ( 5 * 1000)
#define IPERF_UDP_SRV_REP_TIMEOUT  (100)

typedef struct iperf_stream IPERF_STREAM;

typedef void (*IPERF_PROC_CB)(IPERF_STREAM *pStream);


struct iperf_stream
{
    bool                        isActive;
    bool                        isPaused;
    bool                        isStopping;
    bool                        sendStopResponse;
    uint8_t                     state;
    int                         sockfd;
    IPERF_PROC_CB               process;
    IPERF_STREAM                *pParent;
    union
    {
        IPERF_UINT64            strTxBytes;
        IPERF_UINT64            strRxBytes;
    };
    APP_SOCK_ADDR_TYPE    remoteIPAddress;
    APP_SOCK_ADDR_TYPE    localIPAddress;
    uint32_t                    txSeqNum;
    uint32_t                    bytesLastPeriod;
    uint32_t                    rxPackets;
    int32_t                     packetCount;        // How many packets to send for TX test
    uint32_t                    packetByteCount;    // TCP TX
    uint32_t                    packetLength;
    uint32_t                    timeStart;
    uint32_t                    lastTransmission;
    uint32_t                    lastStatsTime;
    uint32_t                    udpBytesToSend;
    uint32_t                    udpBytesToSendMs;
    bool                        useTls;
    int                         tos;
};

typedef struct
{
    IPERF_STREAM                *pParent;
    IPERF_UINT64                strRxBytes;
    APP_SOCK_ADDR_TYPE    ipAddress;
    uint32_t                    nextRxSeqNum;
    uint32_t                    bytesLastPeriod;
    uint32_t                    rxPackets;
    uint32_t                    rxMissingPackets;
    uint16_t                    rxMissingPacketsLastPeriod;
    uint16_t                    rxPacketsLastPeriod;
    uint32_t                    timeStart;
    uint32_t                    lastStatsTime;
    uint16_t                    port;
} IPERF_UDP_RX_STREAM;

typedef struct
{
    UDP_datagram    udpDatagram;
    client_hdr      udpClientHdr;
    uint8_t         msgBuffer[TEST_BUFFER_SIZE];
} IPERF_UDP_MSG;

typedef struct
{
    UDP_datagram    udpDatagram;
    server_hdr      udpServerHdr;
} IPERF_UDP_SRV_MSG;

#define IPERF_MAX_STREAM            5
#define IPERF_MAX_UDP_RX_STREAM     4

static IPERF_STREAM             stream[IPERF_MAX_STREAM];
static IPERF_UDP_RX_STREAM      udpRxStream[IPERF_MAX_UDP_RX_STREAM];
static IPERF_UDP_MSG            udpTxMsg;
static IPERF_UDP_MSG            udpRxMsg;
static IPERF_UDP_SRV_MSG        udpSrvMsg;
static uint8_t*                 pTcpTxMsgBuffer = udpTxMsg.msgBuffer;
static uint8_t*                 pTcpRxMsgBuffer = udpRxMsg.msgBuffer;
static uint32_t                 timeBaseMs;
static uint8_t                  numStreams;
static int                      sessionID;

extern DRV_HANDLE                      wdrvHandle;
//WDRV_WINC_TLS_HANDLE            tlsHandle;
//extern ATCMD_APP_CONTEXT atCmdAppContext;

static uint32_t volatile timerMS;

//------------------------------------------------------------------------------
static uint32_t _getTimeMs(void)
{
    return SYS_TIME_CountToMS(SYS_TIME_CounterGet()) - timeBaseMs;
}

//------------------------------------------------------------------------------
static void* _getPtrToAddr(APP_SOCK_ADDR_TYPE* addr)
{
    if (NULL != addr)
    {
        if (AF_INET == addr->sin_family)
        {
            return (void*)&addr->v4.sin_addr;
        }
        else if (AF_INET6 == addr->sin_family)
        {
            return (void*)&addr->v6.sin6_addr;
        }
    }

    return NULL;
}

//------------------------------------------------------------------------------
static uint32_t _calculateBandwidthKbps(IPERF_UINT64 *pU64, uint32_t time)
{
    uint32_t u32Tmp;

    if (0 == time)
    {
        pU64->high = 0;
        pU64->low = 0;

        return 0;
    }

    if (pU64->high > 0)
    {
        uint32_t n = 32;
        uint32_t mask = 0x80000000;

        while ((pU64->high & mask) == 0)
        {
            n -= 1;
            mask >>= 1;
        }

        u32Tmp = (pU64->high << (32 - n)) | (pU64->low >> n);
        time >>= n;
        u32Tmp /= (time / 8);
        return u32Tmp;
    }
    else if ((pU64->high == 0) && (pU64->low >= 536870912))
    {
        return pU64->low / (time/8);
    }
    else
    {
        return (pU64->low*8) / time;
    }
}

//------------------------------------------------------------------------------
static void _addU64(IPERF_UINT64 *pU64, uint32_t inc)
{
    if (NULL == pU64)
    {
        return;
    }

    pU64->low += inc;

    if (pU64->low < inc)
    {
        pU64->high++;
    }
}

//---------------------------------------------------------------------------
static IPERF_STREAM* _streamAlloc(void)
{
    int i;

    for (i=0; i<IPERF_MAX_STREAM; i++)
    {
        if (false == stream[i].isActive)
        {
            memset(&stream[i], 0, sizeof(IPERF_STREAM));

            stream[i].sockfd = -1;

            numStreams++;

            return &stream[i];
        }
    }

    return NULL;
}

//---------------------------------------------------------------------------
static void _streamFree(IPERF_STREAM *pStream)
{
    if (NULL == pStream)
    {
        return;
    }

    pStream->isActive  = false;
    pStream->state     = 0;
    pStream->pParent   = NULL;
    pStream->sockfd    = -1;

    numStreams--;
}

//---------------------------------------------------------------------------
static IPERF_STREAM* _streamFindSocket(int sockfd)
{
    int i;

    if (-1 == sockfd)
    {
        return NULL;
    }

    for (i=0; i<IPERF_MAX_STREAM; i++)
    {
        if ((true == stream[i].isActive) && (stream[i].sockfd == sockfd))
        {
            return &stream[i];
        }
    }

    return NULL;
}

//------------------------------------------------------------------------------
static void _displayTcpRxStats(IPERF_STREAM *pStream, uint32_t currTimeMs, int startTimeMs)
{
    uint32_t durationMs;
    uint32_t bandwidthKbps;
    uint32_t numKB;
    int endTimeS;
    int startTimeS;

    if (NULL == pStream)
    {
        return;
    }

    durationMs = currTimeMs - pStream->timeStart;

    if (0 == durationMs)
    {
        return;
    }

    if (startTimeMs > 0)
    {
        startTimeS = startTimeMs / 1000;
    }
    else
    {
        startTimeS = 0;
    }

    endTimeS = durationMs + startTimeMs;

    if (endTimeS > 0)
    {
        endTimeS /= 1000;
    }

    bandwidthKbps = _calculateBandwidthKbps(&pStream->strRxBytes, durationMs);
    numKB         = (pStream->strRxBytes.high << 22) | (pStream->strRxBytes.low >> 10);

    SYS_CONSOLE_PRINT("[%3d] Server Report:\r\n", pStream->sockfd);
    SYS_CONSOLE_PRINT("[%3d] %4d-%4d sec %5u KBytes %5u Kbits/sec\r\n", pStream->sockfd, startTimeS, endTimeS, numKB, bandwidthKbps);
}

//------------------------------------------------------------------------------
static void _tcpClientProcess(IPERF_STREAM *pStream)
{
    uint32_t curTimeMs = _getTimeMs();
    if (NULL == pStream)
    {
        return;
    }

    if (true == pStream->isStopping)
    {
        pStream->state = IPERF_TCP_TX_STATE_STOP;
    }

    switch (pStream->state)
    {
        case IPERF_TCP_TX_STATE_CONNECT:
        {
            if (true == pStream->isPaused)
            {
                break;
            }

            pStream->lastTransmission = curTimeMs;

            errno = 0;


            if (-1 == connect(pStream->sockfd, (struct sockaddr*)&pStream->remoteIPAddress, sizeof(APP_SOCK_ADDR_TYPE)))
            {
                if (EINPROGRESS != errno)
                {
                    SYS_CONSOLE_PRINT("[%3d] connect() - %d\r\n", pStream->sockfd, errno);
                    pStream->state = IPERF_TCP_TX_STATE_STOP;
                    break;
                }

                setsockopt(pStream->sockfd, IPPROTO_IP, IP_TOS, &pStream->tos, sizeof(int));

                pStream->state = IPERF_TCP_TX_STATE_CONNECTING;
            }
            else
            {
                pStream->state = IPERF_TCP_TX_STATE_CONNECTED;
            }
            break;
        }

        case IPERF_TCP_TX_STATE_CONNECTING:
        {
            if ((curTimeMs - pStream->lastTransmission) > IPERF_CONNECTION_TIMEOUT)
            {
                SYS_CONSOLE_PRINT("[%3d] connection timeout\r\n", pStream->sockfd);
                pStream->state = IPERF_TCP_TX_STATE_STOP;
            }
            break;
        }

        case IPERF_TCP_TX_STATE_CONNECTED:
        {
            char ipStr[64];
            int sndBufSz;

            inet_ntop(pStream->localIPAddress.sin_family, _getPtrToAddr(&pStream->localIPAddress), ipStr, sizeof(ipStr));
            SYS_CONSOLE_PRINT("[%3d] local %s port %d connected with", pStream->sockfd, ipStr, ntohs(pStream->localIPAddress.sin_port));

            inet_ntop(pStream->remoteIPAddress.sin_family, _getPtrToAddr(&pStream->remoteIPAddress), ipStr, sizeof(ipStr));
            SYS_CONSOLE_PRINT(" %s port %d\r\n", ipStr, ntohs(pStream->remoteIPAddress.sin_port));

            pStream->timeStart        = curTimeMs;
            pStream->lastStatsTime    = pStream->timeStart;
            pStream->state            = IPERF_TCP_TX_STATE_RUNNING;
            pStream->lastTransmission = curTimeMs;

            sndBufSz = (10*1460);
            setsockopt(pStream->sockfd, SOL_SOCKET, SO_SNDBUF, &sndBufSz, sizeof(sndBufSz));

            /* Fall through */
        }

        case IPERF_TCP_TX_STATE_RUNNING:
        {
            ssize_t r;

            do
            {
                errno = 0;

                r = send(pStream->sockfd, (const char*)&pTcpTxMsgBuffer[pStream->packetByteCount], (pStream->packetLength - pStream->packetByteCount), 0);
                if (r > 0)
                {
                    pStream->packetByteCount += (uint32_t)r;
                    pStream->lastTransmission = curTimeMs;
                    

                    if (pStream->packetByteCount == pStream->packetLength)
                    {
                        pStream->packetByteCount = 0;
                        pStream->txSeqNum++;
                        pStream->bytesLastPeriod += pStream->packetLength;
                        _addU64(&pStream->strTxBytes, pStream->packetLength);

                        if (pStream->packetCount > 0)
                        {
                            pStream->packetCount--;

                            if (pStream->packetCount == 0)
                            {
                                pStream->state = IPERF_TCP_TX_STATE_STOP;
                                break;
                            }
                        }
                    }

                    // delta from last print
                    uint32_t timeDelta = curTimeMs - pStream->lastStatsTime;
                    if (timeDelta >= 1000)
                    {
                        uint32_t bandwidthKbps  = (pStream->bytesLastPeriod * 8) / timeDelta;
                        uint32_t numKB          = pStream->bytesLastPeriod / 1024;
                        uint32_t t1S            = (pStream->lastStatsTime - pStream->timeStart) / 1000;
                        uint32_t t2S            = (curTimeMs - pStream->timeStart) / 1000;

                        SYS_CONSOLE_PRINT("[%3d] %4d-%4u sec %5u KBytes %5u Kbits/sec\r\n", pStream->sockfd, t1S, t2S, numKB, bandwidthKbps);

                        pStream->bytesLastPeriod = 0;
                        pStream->lastStatsTime += 1000;
                    }
                }
                else if ((r < 0) && (errno != EWOULDBLOCK) && (errno != EAGAIN))
                {
                    SYS_CONSOLE_PRINT("[%3d] send - %d\r\n", pStream->sockfd, errno);
                    pStream->state = IPERF_TCP_TX_STATE_STOP;
                    break;
                }
                else
                {
                    //SYS_CONSOLE_PRINT("send - %d\r\n", errno);
                }
            }
            while (r > 0);

            if ((curTimeMs - pStream->lastTransmission) > IPERF_TX_TIMEOUT)
            {
                SYS_CONSOLE_PRINT("[%3d] tx timeout (%d,%d, %u)\r\n", pStream->sockfd, r, errno, curTimeMs);
                pStream->state = IPERF_TCP_TX_STATE_STOP;
                break;
            }

            break;
        }

        case IPERF_TCP_TX_STATE_STOP:
        {
            SYS_CONSOLE_PRINT("[%3d] Client Report:\r\n", pStream->sockfd);

            if (pStream->timeStart == 0)
            {
                SYS_CONSOLE_PRINT("[%3d]    Test was aborted by error\r\n", pStream->sockfd);
            }
            else
            {
                uint32_t durationMs     = curTimeMs - pStream->timeStart;
                uint32_t bandwidthKbps  = _calculateBandwidthKbps(&pStream->strTxBytes, durationMs);
                uint32_t numKB          = (pStream->strTxBytes.high << 22) | (pStream->strTxBytes.low >> 10);
                uint32_t durationS      = durationMs / 1000;

                SYS_CONSOLE_PRINT("[%3d]    0-%4u sec %5u KBytes %5u Kbits/sec\r\n", pStream->sockfd, durationS, numKB, bandwidthKbps);
            }

            if (pStream->sockfd >= 0)
            {
                SYS_CONSOLE_PRINT("Closing Socket %d\r\n", pStream->sockfd);
                SYS_CONSOLE_PRINT("Sock to delete <%d>\r\n", pStream->sockfd);

                if (0 != shutdown(pStream->sockfd, SHUT_RDWR))
                {
                    SYS_CONSOLE_PRINT("Socket <%d> failed to close!\r\n", pStream->sockfd);
                }

                if (true == pStream->sendStopResponse)
                {
                    SYS_CONSOLE_PRINT("\r\n+OK\r\n,PIPERF_STOP\r\n");
                }
            }

            _streamFree(pStream);
            break;
        }
    }
}

//------------------------------------------------------------------------------
static void _tcpServerProcess(IPERF_STREAM *pStream)
{
    uint32_t curTimeMs = _getTimeMs();
    if (NULL == pStream)
    {
        return;
    }

    if (true == pStream->isStopping)
    {
        pStream->state = IPERF_TCP_RX_STATE_STOP;
    }

    switch (pStream->state)
    {
        case IPERF_TCP_RX_STATE_BIND:
        {
            errno = 0;

            if (-1 == bind(pStream->sockfd, (struct sockaddr*)&pStream->localIPAddress, sizeof(APP_SOCK_ADDR_TYPE)))
            {
                SYS_CONSOLE_PRINT("[%3d] bind() - %d\r\n", pStream->sockfd, errno);
                pStream->state = IPERF_TCP_RX_STATE_STOP;
                break;
            }

            listen(pStream->sockfd, 0);

            pStream->state = IPERF_TCP_RX_STATE_RUNNING_LISTENER;
            break;
        }

        case IPERF_TCP_RX_STATE_RUNNING_LISTENER:
        {
            int sockfd;
            APP_SOCK_ADDR_TYPE addr;
            socklen_t addrLen = sizeof(APP_SOCK_ADDR_TYPE);
            char ipStr[64];
            IPERF_STREAM *p;
            int rcvBufSz;

            sockfd = accept(pStream->sockfd, (struct sockaddr*)&addr, &addrLen);

            if (-1 == sockfd)
            {
                break;
            }

            inet_ntop(pStream->localIPAddress.sin_family, _getPtrToAddr(&pStream->localIPAddress), ipStr, sizeof(ipStr));
            SYS_CONSOLE_PRINT("[%3d] local %s port %d connected with", sockfd, ipStr, ntohs(pStream->localIPAddress.sin_port));

            inet_ntop(addr.sin_family, _getPtrToAddr(&addr), ipStr, sizeof(ipStr));
            SYS_CONSOLE_PRINT(" %s port %d\r\n", ipStr, ntohs(addr.sin_port));

            p = _streamAlloc();

            if (NULL == p)
            {
                shutdown(sockfd, SHUT_RDWR);
                break;
            }

            p->sockfd           = sockfd;
            p->pParent          = pStream;
            p->isActive         = true;
            p->timeStart        = curTimeMs;
            p->lastStatsTime    = curTimeMs;
            p->state            = IPERF_TCP_RX_STATE_RUNNING_CLIENT;
            p->process          = _tcpServerProcess;

            memcpy(&p->localIPAddress, &pStream->localIPAddress, sizeof(APP_SOCK_ADDR_TYPE));
            memcpy(&p->remoteIPAddress, &addr, sizeof(APP_SOCK_ADDR_TYPE));

            if (pStream->timeStart == 0)
            {
                pStream->timeStart     = curTimeMs;
                pStream->lastStatsTime = curTimeMs;
            }

            rcvBufSz = (10*1460);
            setsockopt(p->sockfd, SOL_SOCKET, SO_RCVBUF, &rcvBufSz, sizeof(rcvBufSz));

            break;
        }

        case IPERF_TCP_RX_STATE_RUNNING_CLIENT:
        {
            ssize_t numBytes;
            ssize_t r;

            errno = 0;
            numBytes = recv(pStream->sockfd, NULL, 0, MSG_PEEK|MSG_TRUNC);
            if ((numBytes <= 0) && (errno != EWOULDBLOCK))
            {
                SYS_CONSOLE_PRINT("[%3d] recv - %d\r\n", pStream->sockfd, errno);

                pStream->state = IPERF_TCP_RX_STATE_STOP;
                break;
            }

            while (numBytes > 0)
            {
                errno = 0;
                r = recv(pStream->sockfd, (char*)pTcpRxMsgBuffer, TEST_BUFFER_SIZE, 0);
                if (r <= 0)
                {
                    if (errno != EWOULDBLOCK)
                    {
                        SYS_CONSOLE_PRINT("[%3d] recv - %d\r\n", pStream->sockfd, errno);
                        pStream->state = IPERF_TCP_RX_STATE_STOP;
                    }
                    break;
                }
                else
                {
                    _addU64(&pStream->pParent->strRxBytes, r);
                    _addU64(&pStream->strRxBytes, r);
                    pStream->bytesLastPeriod += r;
                    pStream->packetByteCount += r;

                    if (pStream->packetByteCount >= pStream->packetLength)
                    {
                        pStream->rxPackets++;
                        pStream->packetByteCount -= pStream->packetLength;
                    }

                    if (r <= numBytes)
                    {
                        numBytes -= r;
                    }
                    else
                    {
                        break;
                    }
                }
            }

            uint32_t timeDelta = curTimeMs - pStream->lastStatsTime;

            if (timeDelta >= 1000)
            {
                uint32_t bandwidthKbps;
                uint32_t numKB;

                if (pStream->bytesLastPeriod > 0)
                {
                    bandwidthKbps   = (pStream->bytesLastPeriod * 8) / timeDelta;
                    numKB           = pStream->bytesLastPeriod / 1024;
                }
                else
                {
                    bandwidthKbps   = 0;
                    numKB           = 0;
                }

                int t1S = (pStream->lastStatsTime - pStream->timeStart) / 1000;
                int t2S = (curTimeMs - pStream->timeStart) / 1000;

                SYS_CONSOLE_PRINT("[%3d] %4d-%4d sec %5u KBytes %5u Kbits/sec\r\n", pStream->sockfd, t1S, t2S, numKB, bandwidthKbps);

                pStream->bytesLastPeriod = 0;
                pStream->lastStatsTime  += 1000;
            }

            break;
        }

        case IPERF_TCP_RX_STATE_STOP:
        {
            // Print the server report for the parent sock.
            _displayTcpRxStats(pStream, curTimeMs, 0);

            if (pStream->pParent == NULL)
            {
                int i;

                // Print the server report for client socks.
                for (i=0; i<IPERF_MAX_STREAM; i++)
                {
                    if (stream[i].pParent == pStream)
                    {
                        _displayTcpRxStats(&stream[i], curTimeMs, stream[i].timeStart - pStream->timeStart);

                        shutdown(stream[i].sockfd, SHUT_RDWR);

                        _streamFree(&stream[i]);
                    }
                }
            }

            if (-1 != pStream->sockfd)
            {
                SYS_CONSOLE_PRINT("Closing Socket %d\r\n", pStream->sockfd);
                SYS_CONSOLE_PRINT("Sock to delete <%d>\r\n", pStream->sockfd);

                if (0 != shutdown(pStream->sockfd, SHUT_RDWR))
                {
                    SYS_CONSOLE_PRINT("Socket <%d> failed to close!\r\n", pStream->sockfd);
                }

                if (true == pStream->sendStopResponse)
                {
                    SYS_CONSOLE_PRINT("\r\n+OK\r\n,PIPERF_STOP\r\n");
                }
            }

            _streamFree(pStream);
            break;
        }
    }
}

//---------------------------------------------------------------------------
static void _udpClientProcess(IPERF_STREAM *pStream)
{
    int r;
    uint32_t curTimeMs = _getTimeMs();

    if (NULL == pStream)
    {
        return;
    }

    if (true == pStream->isStopping)
    {
        if (pStream->state < IPERF_UDP_TX_STATE_RUNNING)
        {
            pStream->state = IPERF_UDP_TX_STATE_STOP;
        }
        else if (pStream->state == IPERF_UDP_TX_STATE_RUNNING)
        {
            pStream->packetCount = 20;
            pStream->state = IPERF_UDP_TX_STATE_SEND_CLOSE_REPORT;
        }
    }

    switch (pStream->state)
    {
        case IPERF_UDP_TX_STATE_CONNECT:
        {
            char ipStr[64];

            /* Report connection details. */

            inet_ntop(pStream->localIPAddress.sin_family, _getPtrToAddr(&pStream->localIPAddress), ipStr, sizeof(ipStr));
            SYS_CONSOLE_PRINT("[%3d] local %s port %d connected with", pStream->sockfd, ipStr, ntohs(pStream->localIPAddress.sin_port));

            inet_ntop(pStream->remoteIPAddress.sin_family, _getPtrToAddr(&pStream->remoteIPAddress), ipStr, sizeof(ipStr));
            SYS_CONSOLE_PRINT(" %s port %d\r\n", ipStr, ntohs(pStream->remoteIPAddress.sin_port));

            pStream->state = IPERF_UDP_TX_STATE_HOLD;

            /* Fall through */
        }

        case IPERF_UDP_TX_STATE_HOLD:
        {
            if (false == pStream->isPaused)
            {
                pStream->timeStart        = curTimeMs;
                pStream->lastStatsTime    = curTimeMs;
                pStream->udpBytesToSend   = pStream->udpBytesToSendMs;
                pStream->lastTransmission = curTimeMs;

                if (-1 == connect(pStream->sockfd, (const struct sockaddr*)&pStream->remoteIPAddress, sizeof(APP_SOCK_ADDR_TYPE)))
                {
                    if (EINPROGRESS != errno)
                    {
                        SYS_CONSOLE_PRINT("[%3d] connect() - %d\r\n", pStream->sockfd, errno);
                        pStream->state = IPERF_UDP_TX_STATE_STOP;
                        break;
                    }
                }

                setsockopt(pStream->sockfd, IPPROTO_IP, IP_TOS, &pStream->tos, sizeof(int));

                pStream->state = IPERF_UDP_TX_STATE_RUNNING;
            }
            break;
        }

        case IPERF_UDP_TX_STATE_RUNNING:
        {
            uint32_t curTimeSec = curTimeMs / 1000;
            uint32_t curTimeUs  = (curTimeMs % 1000) * 1000;
            uint32_t timeDelta = curTimeMs - pStream->lastTransmission;
            uint32_t udpBytesToSend = 0;
            bool bSendSuccess = false;

            if (pStream->udpBytesToSendMs > 0)
            {
                /* If rate limited calculate the outstanding bytes to send based on previous unsent
                 data and new unsent data based on the delta since last transmission. */
                udpBytesToSend = pStream->udpBytesToSend + (timeDelta * pStream->udpBytesToSendMs);

                /* Watch for wrap around, indicates long run with requested data rate higher than
                 can be achieved. */
                if (udpBytesToSend < pStream->udpBytesToSend)
                {
                    udpBytesToSend = pStream->udpBytesToSend;
                }
            }

            /* If rate limited, check if unsent data makes up one complete packet. */
            if ((0 == pStream->udpBytesToSendMs) || (udpBytesToSend >= pStream->packetLength))
            {
                // For UDP Client: store datagram ID into the packet header
                udpTxMsg.udpDatagram.id      = htonl(pStream->txSeqNum);
                udpTxMsg.udpDatagram.tv_sec  = htonl(curTimeSec);
                udpTxMsg.udpDatagram.tv_usec = htonl(curTimeUs);
                udpTxMsg.udpDatagram.id2     = 0;
                udpTxMsg.udpClientHdr.flags  = 0;

                errno = 0;

                r = sendto(pStream->sockfd, (const char*)&udpTxMsg, pStream->packetLength, 0, (const struct sockaddr*)&pStream->remoteIPAddress, sizeof(APP_SOCK_ADDR_TYPE));

                if (r > 0)
                {
                    if (pStream->udpBytesToSendMs > 0)
                    {
                        udpBytesToSend -= pStream->packetLength;
                    }

                    /* Update last transmission timestamp, set flag to indicate we have updated it. */
                    pStream->lastTransmission = curTimeMs;
                    bSendSuccess = true;

                    _addU64(&pStream->strTxBytes, pStream->packetLength);

                    pStream->txSeqNum++;
                    pStream->bytesLastPeriod += pStream->packetLength;

                    if (pStream->packetCount > 0)
                    {
                        pStream->packetCount--;

                        if (pStream->packetCount == 0)
                        {
                            pStream->packetCount = 20;
                            pStream->state = IPERF_UDP_TX_STATE_SEND_CLOSE_REPORT;
                            break;
                        }
                    }
                }
                else if ((r < 0) && (errno != EWOULDBLOCK))
                {
                    /* Send failed, other than just blocked. */
                    SYS_CONSOLE_PRINT("[%3d] sendto() - %d\r\n", pStream->sockfd, errno);
                    pStream->state = IPERF_UDP_TX_STATE_STOP;
                }
            }

            if (true == bSendSuccess)
            {
                /* If we've sent at least one packet and updated last transmission timestamp
                 store the outstanding unsent bytes count for next time. */
                pStream->udpBytesToSend = udpBytesToSend;
            }

            if (IPERF_UDP_TX_STATE_CONNECT == pStream->state)
            {
                if (timeDelta > IPERF_CONNECTION_TIMEOUT)
                {
                    SYS_CONSOLE_PRINT("[%3d] connection timeout\r\n", pStream->sockfd);
                    pStream->state = IPERF_UDP_TX_STATE_STOP;
                }
            }
            else
            {
                if (timeDelta > IPERF_TX_TIMEOUT)
                {
                    SYS_CONSOLE_PRINT("[%3d] tx timeout\r\n", pStream->sockfd);
                    pStream->state = IPERF_UDP_TX_STATE_STOP;
                }
            }

            if (IPERF_UDP_TX_STATE_STOP == pStream->state)
            {
                break;
            }

            // delta from last stats print
            timeDelta = curTimeMs - pStream->lastStatsTime;
            if (timeDelta >= 1000)
            {
                uint32_t bandwidthKbps  = (pStream->bytesLastPeriod * 8) / timeDelta;
                uint32_t numKB          = pStream->bytesLastPeriod / 1024;
                uint32_t time1S         = (pStream->lastStatsTime - pStream->timeStart) / 1000;
                uint32_t time2S         = (curTimeMs - pStream->timeStart) / 1000;

                SYS_CONSOLE_PRINT("[%3d] %4d-%4u sec %5u KBytes %5u Kbits/sec\r\n", pStream->sockfd, time1S, time2S, numKB, bandwidthKbps);

                pStream->bytesLastPeriod = 0;
                pStream->lastStatsTime += 1000;
            }

            break;
        }

        case IPERF_UDP_TX_STATE_SEND_CLOSE_REPORT:
        {
            uint32_t curTimeSec = curTimeMs / 1000;
            uint32_t curTimeUs  = (curTimeMs % 1000) * 1000;

            // Store Sequence Number into the datagram
            udpTxMsg.udpDatagram.id      = htonl(-pStream->txSeqNum);
            udpTxMsg.udpDatagram.tv_sec  = htonl(curTimeSec);
            udpTxMsg.udpDatagram.tv_usec = htonl(curTimeUs);
            udpTxMsg.udpDatagram.id2     = 0xffffffff;
            udpTxMsg.udpClientHdr.flags  = 0;

            errno = 0;
            r = sendto(pStream->sockfd, (const char*)&udpTxMsg, pStream->packetLength, 0, (const struct sockaddr*)&pStream->remoteIPAddress, sizeof(APP_SOCK_ADDR_TYPE));

            if (r > 0)
            {
                pStream->txSeqNum++;
                pStream->lastTransmission = curTimeMs;
                pStream->state = IPERF_UDP_TX_STATE_WAIT_FOR_SERVER_REPORT;
            }
            else if (errno != EWOULDBLOCK)
            {
                SYS_CONSOLE_PRINT("[%3d] send - %d\r\n", pStream->sockfd, errno);
                SYS_CONSOLE_PRINT("[%3d]    Failed to send Close Report\r\n", pStream->sockfd);

                pStream->state = IPERF_UDP_TX_STATE_STOP;
            }
            else if ((curTimeMs - pStream->lastTransmission) > IPERF_TX_TIMEOUT)
            {
                SYS_CONSOLE_PRINT("[%3d] tx timeout - Failed to send close report\r\n", pStream->sockfd);

                pStream->state = IPERF_UDP_TX_STATE_STOP;
                pStream->timeStart = 0;
            }
            break;
        }

        case IPERF_UDP_TX_STATE_WAIT_FOR_SERVER_REPORT:
        {
            // After the UDP Client completes the data transfer, the server
            // sends a Report to the client.
            APP_SOCK_ADDR_TYPE addr;
            socklen_t fromLen = sizeof(APP_SOCK_ADDR_TYPE);

            errno = 0;
            r = recvfrom(pStream->sockfd, (char *)&udpRxMsg, TEST_BUFFER_SIZE, 0, (struct sockaddr*)&addr, &fromLen);

            if (r > 0)
            {
                server_hdr * report = (server_hdr*)(((uint8_t*)&udpRxMsg) + sizeof(struct UDP_datagram));

                if (ntohl(report->flags) == 0x88000000)
                {
                    IPERF_UINT64 strRxBytes;

                    strRxBytes.high = ntohl(report->total_len1);
                    strRxBytes.low  = ntohl(report->total_len2);

                    uint32_t timeDelta      = (ntohl(report->stop_sec) * 1000) + (ntohl(report->stop_usec)) /1000;
                    uint32_t bandwidthKbps  = _calculateBandwidthKbps(&strRxBytes, timeDelta);
                    uint32_t numKB          = (strRxBytes.high << 22) | (strRxBytes.low >> 10);
                    uint32_t durationS      = ntohl(report->stop_sec);
                    uint32_t error          = ntohl(report->error_cnt);
                    uint32_t datagrams      = ntohl(report->datagrams);

                    SYS_CONSOLE_PRINT("[%3d] Server Report:\r\n", pStream->sockfd);
                    SYS_CONSOLE_PRINT("[%3d]    0-%4u sec %5u KBytes %5u Kbits/sec %5u/%5u\r\n", pStream->sockfd, durationS, numKB, bandwidthKbps, error, datagrams);
                    SYS_CONSOLE_PRINT("[%3d] Sent %u datagrams\r\n", pStream->sockfd, datagrams);

                    pStream->state = IPERF_UDP_TX_STATE_STOP;
                }
            }
            else if (errno != EWOULDBLOCK)
            {
                SYS_CONSOLE_PRINT("[%3d] recv - %d\r\n", pStream->sockfd, errno);

                pStream->state = IPERF_UDP_TX_STATE_STOP;
            }
            else if ((curTimeMs - pStream->lastTransmission) > IPERF_UDP_SRV_REP_TIMEOUT)
            {
                pStream->packetCount--;

                if (pStream->packetCount == 0)
                {
                    SYS_CONSOLE_PRINT("[%3d] rx timeout - Failed to receive server report\r\n", pStream->sockfd);

                    pStream->state = IPERF_UDP_TX_STATE_STOP;
                }
                else
                {
                    pStream->state = IPERF_UDP_TX_STATE_SEND_CLOSE_REPORT;
                }
            }

            break;
        }

        case IPERF_UDP_TX_STATE_STOP:
        {
            SYS_CONSOLE_PRINT("[%3d] Client Report:\r\n", pStream->sockfd);

            if (pStream->timeStart == 0)
            {
                SYS_CONSOLE_PRINT("[%3d]    Test was aborted by error\r\n", pStream->sockfd);
            }
            else
            {
                // FIXME:
                // Throughput will be inaccurate if any error happens at
                // SEND_CLOSE_REPORT or WAIT_FOR_SERVER_REPOR state
                //

                uint32_t durationMs     = curTimeMs - pStream->timeStart;
                uint32_t bandwidthKbps  = _calculateBandwidthKbps(&pStream->strTxBytes, durationMs);
                uint32_t numKB          = (pStream->strTxBytes.high << 22) | (pStream->strTxBytes.low >> 10);
                uint32_t durationS      = durationMs / 1000;

                SYS_CONSOLE_PRINT("[%3d]    0-%4u sec %5u KBytes %5u Kbits/sec\r\n", pStream->sockfd, durationS, numKB, bandwidthKbps);
            }

            if (pStream->sockfd != -1)
            {
                SYS_CONSOLE_PRINT("Closing Socket %d\r\n", pStream->sockfd);
                SYS_CONSOLE_PRINT("Sock to delete <%d>\r\n", pStream->sockfd);

                if (0 != shutdown(pStream->sockfd, SHUT_RDWR))
                {
                    SYS_CONSOLE_PRINT("Socket <%d> failed to close!\r\n", pStream->sockfd);
                }

                if (true == pStream->sendStopResponse)
                {
                    SYS_CONSOLE_PRINT("\r\n+OK\r\n,PIPERF_STOP\r\n");
                    
                }
            }

            _streamFree(pStream);

            break;
        }
    }
}

//------------------------------------------------------------------------------
static void _udpServerProcess(IPERF_STREAM *pStream)
{
    uint32_t curTimeMs = _getTimeMs();
    if (NULL == pStream)
    {
        return;
    }

    if (true == pStream->isStopping)
    {
        pStream->state = IPERF_UDP_RX_STATE_STOP;
    }

    switch (pStream->state)
    {
        case IPERF_UDP_RX_STATE_BIND:
        {
            errno = 0;

            if (-1 == bind(pStream->sockfd, (struct sockaddr*)&pStream->localIPAddress, sizeof(APP_SOCK_ADDR_TYPE)))
            {
                SYS_CONSOLE_PRINT("[%3d] bind() - %d\r\n", pStream->sockfd, errno);
                pStream->state = IPERF_UDP_RX_STATE_STOP;
                break;
            }

            pStream->state = IPERF_UDP_RX_STATE_RUNNING;
            break;
        }

        case IPERF_UDP_RX_STATE_RUNNING:
        {
            int i;
            int r;
            IPERF_UDP_RX_STREAM *pUdpStream = NULL;

            do
            {
                curTimeMs = _getTimeMs();

                errno = 0;
                r = recv(pStream->sockfd, NULL, 0, MSG_PEEK|MSG_TRUNC);
                if (-1 == r)
                {
                    if (errno != EWOULDBLOCK)
                    {
                        SYS_CONSOLE_PRINT("[%d] recv - %d\r\n", pStream->sockfd, errno);
                        pStream->state = IPERF_UDP_RX_STATE_STOP;
                        break;
                    }
                }
                else if (r >= (sizeof(UDP_datagram) + sizeof(client_hdr)))
                {
                    APP_SOCK_ADDR_TYPE addr;
                    socklen_t fromlen = sizeof(APP_SOCK_ADDR_TYPE);
                    uint32_t id;

                    errno = 0;
                    r = recvfrom(pStream->sockfd, (char *)&udpRxMsg, TEST_BUFFER_SIZE, 0, (struct sockaddr*)&addr, &fromlen);
                    if (-1 == r)
                    {
                        SYS_CONSOLE_PRINT("[%d] recvfrom - %d\r\n", pStream->sockfd, errno);
                        pStream->state = IPERF_UDP_RX_STATE_STOP;
                        break;
                    }

                    pUdpStream = NULL;
                    for (i=0; i<IPERF_MAX_UDP_RX_STREAM; i++)
                    {
                        if ((udpRxStream[i].pParent == pStream) && (udpRxStream[i].port == addr.sin_port))
                        {
                            pUdpStream = &udpRxStream[i];
                            break;
                        }
                    }

                    id = ntohl(udpRxMsg.udpDatagram.id);

                    if ((id & 0x80000000) == 0x80000000)
                    {
                        if (NULL != pUdpStream)
                        {
                            uint32_t durationMs = curTimeMs - pUdpStream->timeStart;

                            udpSrvMsg.udpDatagram.id            = udpRxMsg.udpDatagram.id;
                            udpSrvMsg.udpDatagram.id2           = udpRxMsg.udpDatagram.id2;
                            udpSrvMsg.udpServerHdr.flags        = htonl(0x88000000);
                            udpSrvMsg.udpServerHdr.datagrams    = htonl(-id);
                            udpSrvMsg.udpServerHdr.error_cnt    = htonl(pUdpStream->rxMissingPackets);
                            udpSrvMsg.udpServerHdr.jitter1      = 0;
                            udpSrvMsg.udpServerHdr.jitter2      = 0;
                            udpSrvMsg.udpServerHdr.outorder_cnt = 0;
                            udpSrvMsg.udpServerHdr.stop_sec     = htonl(durationMs / 1000);
                            udpSrvMsg.udpServerHdr.stop_usec    = htonl(durationMs % 1000 * 1000);
                            udpSrvMsg.udpServerHdr.total_len1   = htonl(pUdpStream->strRxBytes.high);
                            udpSrvMsg.udpServerHdr.total_len2   = htonl(pUdpStream->strRxBytes.low);

                            errno = 0;
                            r = sendto(pStream->sockfd, (const char*)&udpSrvMsg, sizeof(IPERF_UDP_SRV_MSG),
                                    0, (const struct sockaddr*)&pUdpStream->ipAddress, sizeof(APP_SOCK_ADDR_TYPE));
                            if (r <= 0)
                            {
                                if (errno != EWOULDBLOCK)
                                {
                                    SYS_CONSOLE_PRINT("[%d.%d] sendto - %d\r\n", pStream->sockfd, i, errno);
                                }

                                break;
                            }
                            else
                            {
                                pStream->state = IPERF_UDP_RX_STATE_STOP;
                            }
                        }
                    }
                    else
                    {
                        if (pUdpStream == NULL)
                        {
                            char ipStr[64];

                            for (i=0; i <IPERF_MAX_UDP_RX_STREAM; i++)
                            {
                                if (udpRxStream[i].pParent == NULL)
                                {
                                    pUdpStream = &udpRxStream[i];
                                    break;
                                }
                            }

                            inet_ntop(pStream->localIPAddress.sin_family, _getPtrToAddr(&pStream->localIPAddress), ipStr, sizeof(ipStr));
                            SYS_CONSOLE_PRINT("[%3d] local %s port %d connected with", pStream->sockfd, ipStr, ntohs(pStream->localIPAddress.sin_port));

                            inet_ntop(addr.sin_family, _getPtrToAddr(&addr), ipStr, sizeof(ipStr));
                            SYS_CONSOLE_PRINT(" %s port %d\r\n", ipStr, ntohs(addr.sin_port));

                            if (pUdpStream != NULL)
                            {
                                memset((uint8_t*)pUdpStream, 0, sizeof(IPERF_UDP_RX_STREAM));

                                memcpy(&pUdpStream->ipAddress, &addr, sizeof(APP_SOCK_ADDR_TYPE));

                                pUdpStream->pParent       = pStream;
                                pUdpStream->port          = addr.sin_port;
                                pUdpStream->nextRxSeqNum  = 1;
                                pUdpStream->timeStart     = curTimeMs;
                                pUdpStream->lastStatsTime = curTimeMs;

                                if (pStream->timeStart == 0)
                                {
                                    pStream->timeStart = curTimeMs;
                                }
                            }
                            else
                            {
                                SYS_CONSOLE_PRINT("No IPERF_UDP_RX_STREAM allocated\r\n");
                            }
                        }

                        if (pUdpStream != NULL)
                        {
                            if (pUdpStream->nextRxSeqNum > id)
                            {
                                SYS_CONSOLE_PRINT("[%3d] Dup\r\n", pStream->sockfd);
                                break;
                            }

                            if (pUdpStream->nextRxSeqNum != id)
                            {
                                pUdpStream->rxMissingPackets += id - pUdpStream->nextRxSeqNum;
                                pUdpStream->rxMissingPacketsLastPeriod += id - pUdpStream->nextRxSeqNum;
                            }

                            pUdpStream->nextRxSeqNum = id + 1;
                            pUdpStream->rxPackets++;
                            pUdpStream->rxPacketsLastPeriod++;
                            pUdpStream->pParent->rxPackets++;

                            _addU64(&pUdpStream->strRxBytes, r);
                            _addU64(&pUdpStream->pParent->strRxBytes, r);

                            pUdpStream->bytesLastPeriod += (uint32_t)r;
                        }
                    }
                }
            }
            while ((r > 0) && (IPERF_UDP_RX_STATE_STOP == pStream->state));

            // Show Log
            for (i=0; i<IPERF_MAX_UDP_RX_STREAM; i++)
            {
                pUdpStream = &udpRxStream[i];

                if (pUdpStream->pParent == pStream)
                {
                    uint32_t timeDelta = curTimeMs - pUdpStream->lastStatsTime;
                    if (timeDelta >= 1000)
                    {
                        uint32_t bandwidthKbps  = (pUdpStream->bytesLastPeriod * 8) / timeDelta;
                        uint32_t numKB          = pUdpStream->bytesLastPeriod / 1024;
                        uint32_t timeStart      = (pUdpStream->lastStatsTime - pUdpStream->timeStart) / 1000;
                        uint32_t durationS      = (curTimeMs - pUdpStream->timeStart) / 1000;

                        SYS_CONSOLE_PRINT("[%d.%d] %4d-%4d sec %5u KBytes %5u Kbits/sec %5d/%5d\r\n",
                                pStream->sockfd, i, timeStart, durationS, numKB, bandwidthKbps,
                                pUdpStream->rxMissingPacketsLastPeriod, pUdpStream->rxPacketsLastPeriod);

                        pUdpStream->bytesLastPeriod             = 0;
                        pUdpStream->rxMissingPacketsLastPeriod  = 0;
                        pUdpStream->rxPacketsLastPeriod         = 0;
                        pUdpStream->lastStatsTime               += 1000;
                    }
                }
            }
            break;
        }

        case IPERF_UDP_RX_STATE_STOP:
        {
            if (true == pStream->isStopping)
            {
                SYS_CONSOLE_PRINT("Closing Socket %d\r\n", pStream->sockfd);
                SYS_CONSOLE_PRINT("Sock to delete <%d>\r\n", pStream->sockfd);
            }

            if (pStream->timeStart == 0)
            {
                SYS_CONSOLE_PRINT("[%3d]    Test was aborted by error\r\n", pStream->sockfd);
            }
            else
            {
                int i;
                IPERF_UDP_RX_STREAM *pUdpStream;

                uint32_t durationMs     = curTimeMs - pStream->timeStart;
                uint32_t bandwidthKbps  = _calculateBandwidthKbps(&pStream->strRxBytes, durationMs);
                uint32_t numKB          = (pStream->strRxBytes.high << 22) | (pStream->strRxBytes.low >> 10);
                uint32_t durationS      = durationMs / 1000;

                for (i=0; i<IPERF_MAX_UDP_RX_STREAM; i++)
                {
                    pUdpStream = &udpRxStream[i];

                    if (pUdpStream->pParent == pStream)
                    {
                        durationMs      = curTimeMs - pUdpStream->timeStart;
                        bandwidthKbps   = _calculateBandwidthKbps(&pUdpStream->strRxBytes, durationMs);
                        numKB           = (pUdpStream->strRxBytes.high << 22) | (pUdpStream->strRxBytes.low >> 10);
                        durationS       = durationMs / 1000;

                        SYS_CONSOLE_PRINT("[%d.%d] Server Report:\r\n", pStream->sockfd, i);
                        SYS_CONSOLE_PRINT("[%d.%d]    0-%5u sec %5u KBytes %5u Kbits/sec %5u/%5u\r\n",
                                pStream->sockfd, i, durationS, numKB, bandwidthKbps, pUdpStream->rxMissingPackets,
                                pUdpStream->rxPackets);

                        memset((uint8_t*)pUdpStream, 0, sizeof(IPERF_UDP_RX_STREAM));
                    }
                }
            }

            if (true == pStream->isStopping)
            {
                if (pStream->sockfd != -1)
                {
                    if (0 != shutdown(pStream->sockfd, SHUT_RDWR))
                    {
                        SYS_CONSOLE_PRINT("Socket <%d> failed to close!\r\n", pStream->sockfd);
                    }

                    if (true == pStream->sendStopResponse)
                    {
                        SYS_CONSOLE_PRINT("\r\n+OK\r\n,PIPERF_STOP\r\n");
                    }
                }

                _streamFree(pStream);
            }
            else if (-1 != pStream->sockfd)
            {
                pStream->state = IPERF_UDP_RX_STATE_RUNNING;
            }
            else
            {
                _streamFree(pStream);
            }
            break;
        }
    }
}

//------------------------------------------------------------------------------
static bool _tcpClientInit(IPERF_STREAM *pStream, IPERF_INIT_DATA* pInit)
{
    int sockfd;

    if ((NULL == pStream) || (NULL == pInit))
    {
        return false;
    }

    errno = 0;
    sockfd = socket(pInit->remoteIPAddress.sin_family, SOCK_STREAM, pInit->useTls ? IPPROTO_TLS : IPPROTO_TCP);

    if (sockfd < 0)
    {
        SYS_CONSOLE_PRINT("sock error:%d\r\n", errno);

        _streamFree(pStream);
        return false;
    }

    pStream->sockfd             = sockfd;
    pStream->isActive           = true;
    pStream->isPaused           = true;
    pStream->process            = _tcpClientProcess;
    pStream->packetCount        = pInit->packetToSend;
    pStream->packetLength       = pInit->packetLength;
    pStream->state              = IPERF_TCP_TX_STATE_OPENING;
    pStream->useTls             = pInit->useTls ? 1 : 0;

    return true;
}

//------------------------------------------------------------------------------
static bool _tcpServerInit(IPERF_STREAM *pStream, IPERF_INIT_DATA* pInit)
{
    int sockfd;

    if ((NULL == pStream) || (NULL == pInit))
    {
        return false;
    }

    errno = 0;
    sockfd = socket(pInit->localIPAddress.sin_family, SOCK_STREAM, IPPROTO_TCP);

    if (sockfd < 0)
    {
        SYS_CONSOLE_PRINT("sock error:%d\r\n", errno);

        _streamFree(pStream);
        return false;
    }

    pStream->sockfd             = sockfd;
    pStream->isActive           = true;
    pStream->isPaused           = true;
    pStream->process            = _tcpServerProcess;
    pStream->packetCount        = pInit->packetToSend;
    pStream->packetLength       = pInit->packetLength;
    pStream->state              = IPERF_TCP_RX_STATE_OPENING;

    return true;
}

//------------------------------------------------------------------------------
static IPERF_STREAM* _udpClientStart(IPERF_STREAM *pStream, IPERF_INIT_DATA* pInit)
{
    int sockfd;

    if ((NULL == pStream) || (NULL == pInit))
    {
        return false;
    }

    errno = 0;
    sockfd = socket(pInit->localIPAddress.sin_family, SOCK_DGRAM, IPPROTO_UDP);

    if (-1 == sockfd)
    {
        SYS_CONSOLE_PRINT("sock() - %d\r\n", errno);
        _streamFree(pStream);
        return NULL;
    }

    pStream->sockfd             = sockfd;
    pStream->isActive           = true;
    pStream->isPaused           = true;
    pStream->process            = _udpClientProcess;
    pStream->lastTransmission   = _getTimeMs();
    pStream->packetCount        = pInit->packetToSend;
    pStream->packetLength       = pInit->packetLength;
    pStream->udpBytesToSendMs   = (pInit->dataRateBps > 0) ? (pInit->dataRateBps / (8*1000)) : 0;
    pStream->udpBytesToSend     = pStream->packetLength;
    pStream->state              = IPERF_UDP_TX_STATE_OPENING;

    return pStream;
}

//------------------------------------------------------------------------------
static bool _udpServerInit(IPERF_STREAM *pStream, IPERF_INIT_DATA* pInit)
{
    int sockfd;

    if ((NULL == pStream) || (NULL == pInit))
    {
        return false;
    }

    errno = 0;
    sockfd = socket(pInit->localIPAddress.sin_family, SOCK_DGRAM, IPPROTO_UDP);

    if (sockfd < 0)
    {
        SYS_CONSOLE_PRINT("sock error:%d\r\n", errno);

        _streamFree(pStream);
        return false;
    }

    pStream->sockfd             = sockfd;
    pStream->isActive           = true;
    pStream->isPaused           = true;
    pStream->process            = _udpServerProcess;
    pStream->packetCount        = pInit->packetToSend;
    pStream->packetLength       = pInit->packetLength;
    pStream->state              = IPERF_UDP_RX_STATE_OPENING;

    return true;
}

//------------------------------------------------------------------------------
static void _SocketEventCallback(uintptr_t context, int sockfd, WINC_SOCKET_EVENT event, WINC_SOCKET_STATUS status)
{
    IPERF_STREAM *pStream = _streamFindSocket(sockfd);

    if (NULL == pStream)
    {
        return;
    }

    if (WINC_SOCKET_STATUS_OK != status)
    {
    }

    switch (event)
    {
        case WINC_SOCKET_EVENT_OPEN:
        {
            if (IPERF_TCP_TX_STATE_OPENING == pStream->state)
            {
                pStream->state = IPERF_TCP_TX_STATE_CONNECT;
            }
            else if (IPERF_TCP_RX_STATE_OPENING == pStream->state)
            {
                pStream->state = IPERF_TCP_RX_STATE_BIND;
            }
            else if (IPERF_UDP_TX_STATE_OPENING == pStream->state)
            {
                pStream->state = IPERF_UDP_TX_STATE_CONNECT;
            }
            else if (IPERF_UDP_RX_STATE_OPENING == pStream->state)
            {
                pStream->state = IPERF_UDP_RX_STATE_BIND;
            }

            break;
        }

        case WINC_SOCKET_EVENT_CONNECT:
        {
            if (IPERF_TCP_TX_STATE_CONNECTING == pStream->state)
            {
                pStream->state = IPERF_TCP_TX_STATE_CONNECTED;
            }

            break;
        }

        default:
        {
            break;
        }
    }
}

//------------------------------------------------------------------------------
static IPERF_STREAM* _streamCreate(IPERF_INIT_DATA* pInit)
{
    IPERF_STREAM *pStream;
    bool streamCreated = false;

    if (NULL == pInit)
    {
        return NULL;
    }

    pStream = _streamAlloc();

    if (NULL == pStream)
    {
        return NULL;
    }

    memcpy(&pStream->localIPAddress, &pInit->localIPAddress, sizeof(APP_SOCK_ADDR_TYPE));
    memcpy(&pStream->remoteIPAddress, &pInit->remoteIPAddress, sizeof(APP_SOCK_ADDR_TYPE));

    switch (pInit->opMode)
    {
        case MODE_TCP_CLIENT:
        {
            streamCreated = _tcpClientInit(pStream, pInit);
            break;
        }

        case MODE_TCP_SERVER:
        {
            streamCreated = _tcpServerInit(pStream, pInit);
            break;
        }

        case MODE_UDP_CLIENT:
        {
            streamCreated = _udpClientStart(pStream, pInit);
            break;
        }

        case MODE_UDP_SERVER:
        {
            streamCreated = _udpServerInit(pStream, pInit);
            break;
        }

        default:
        {
            SYS_CONSOLE_PRINT("Invalid Parameter: operating_mode:%d\r\n", pInit->opMode);
            break;
        }
    }

    if (false == streamCreated)
    {
        _streamFree(pStream);
    }

    if (NULL != pStream)
    {
        SYS_CONSOLE_PRINT("iPerf Socket %d session ID = %d\r\n", pStream->sockfd, ++sessionID);
    }

    return pStream;
}

//------------------------------------------------------------------------------
static bool _streamStart(IPERF_STREAM *pStream)
{
    if (NULL == pStream)
    {
        return false;
    }

    // Reset Timer on the first instance creation.
    if (numStreams == 0)
    {
        timeBaseMs = SYS_TIME_CountToMS(SYS_TIME_CounterGet());
    }

    if (true == pStream->isActive)
    {
        pStream->isPaused = false;
    }

    return true;
}

//------------------------------------------------------------------------------
static void _streamUpdate(IPERF_STREAM *pStream)
{
    if (NULL == pStream)
    {
        return;
    }

    if ((true == pStream->isActive) && (NULL != pStream->process))
    {
        pStream->process(pStream);
    }
}

//------------------------------------------------------------------------------
static bool _streamStop(IPERF_STREAM *pStream, bool report)
{
    if (NULL == pStream)
    {
        return false;
    }

    if (true == pStream->isActive)
    {
        pStream->isStopping = true;

        if (true == report)
        {
            pStream->sendStopResponse = true;
        }
    }

    return true;
}

//------------------------------------------------------------------------------
uint8_t IPERF_GetNumActiveSockets(void)
{
    return numStreams;
}

//------------------------------------------------------------------------------
void IPERF_Init(void)
{
    uint16_t len = TEST_BUFFER_SIZE;
    int i;

    sessionID  = 0;
    numStreams = 0;
    memset((uint8_t*)&stream, 0, sizeof(stream));

    WDRV_WINC_SocketRegisterEventCallback(wdrvHandle, &_SocketEventCallback);

    for (i=0; i<IPERF_MAX_STREAM; i++)
    {
        stream[i].sockfd = -1;
    }

    while(len-- > 0)
    {
        pTcpTxMsgBuffer[len] = (len % 10) + '0';
    }
}

//------------------------------------------------------------------------------
bool IPERF_Create(IPERF_INIT_DATA* pInit, bool isPaused)
{
    IPERF_STREAM *pStream;

    pStream = _streamCreate(pInit);

    if (NULL == pStream)
    {
        return false;
    }

    if (isPaused == false)
    {
        return _streamStart(pStream);
    }

    return true;
}

//------------------------------------------------------------------------------
bool IPERF_Start(void)
{
    int i;

    for (i=0; i<IPERF_MAX_STREAM; i++)
    {
        if (false == _streamStart(&stream[i]))
        {
            return false;
        }
    }

    return true;
}

//------------------------------------------------------------------------------
bool IPERF_Stop(int sockfd)
{
    int i;

    if (-1 == sockfd)
    {
        for (i=0; i<IPERF_MAX_STREAM; i++)
        {
            if (false == _streamStop(&stream[i], false))
            {
                return false;
            }
        }
    }
    else
    {
        for (i=0; i<IPERF_MAX_STREAM; i++)
        {
            if (stream[i].sockfd == sockfd)
            {
                return _streamStop(&stream[i], true);
            }
        }
    }

    return true;
}

//------------------------------------------------------------------------------
void IPERF_Update(void)
{
    int i;

    for (i=0; i<IPERF_MAX_STREAM; i++)
    {
        _streamUpdate(&stream[i]);
    }
}

bool IPERF_setStreamTos(int sockfd, uint8_t tid)
{
    int i;
    for (i=0; i<IPERF_MAX_STREAM; i++)
    {
        if (stream[i].sockfd == sockfd)
        {
            switch(tid)
            {
                case 3:     // BACKGROUND
                {
                    stream[i].tos = 0x04;
                    break;
                }

                case 2:     // Voice
                {
                    stream[i].tos = 0x08;
                    break;
                }

                case 1:     // Video
                {
                    stream[i].tos = 0x10;
                    break;
                }

                case 0:     // Best Effort
                default:
                {
                    stream[i].tos = 0;
                    break;
                }
            }
            return true;
        }
    }
    return false;
}

