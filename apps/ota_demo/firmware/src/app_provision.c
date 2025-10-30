
/*******************************************************************************
Copyright (C) 2024 released Microchip Technology Inc. All rights reserved.

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


/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */


/* This section lists the other files that are included in this file.
 */
#include "app_provision.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// Global data for Provision application
APP_PROV_CONTEXT g_appProvCtx;


static void APP_PROV_CmdPrintUsage (void)
{
    SYS_CONSOLE_PRINT(TERM_YELLOW"\r\n*********************** COMMAND HELP ************************************\r\n"TERM_RESET);
    SYS_CONSOLE_PRINT(">> Command     : prov \r\n");
    SYS_CONSOLE_PRINT(">> Description : Start and Stop of Device Provision mode \r\n");
    SYS_CONSOLE_PRINT(">> Options     : start -> Start the provision\r\n"
                      "               : stop  -> Stop the provision\r\n"
                      "               : help  -> Help Command\r\n\r\n");
    SYS_CONSOLE_PRINT(">> Syntax      : prov <REG_DOMAIN> start\r\n"
            "\t\t\tREG_DOMAIN   : GEN, USA, EMEA \r\n\r\n");
    SYS_CONSOLE_PRINT(">> Syntax      : prov stop\r\n\r\n");
    SYS_CONSOLE_PRINT(">> Syntax      : prov help\r\n\r\n");
    SYS_CONSOLE_PRINT(TERM_YELLOW"\r\n**************************************************************************\r\n"TERM_RESET);
    return;
}

APP_PROV_STATES APP_PROV_GetProvStatus( void )
{
    return g_appProvCtx.state;
}
// *****************************************************************************
// *****************************************************************************
// Function: APP_PROV_CmdProcessing
//
// Summary:
//    Processes provisioning commands.
//
// Description:
//    This function processes commands related to the provisioning application. It
//    takes the command input, parses it, and performs the necessary actions.
//
// Parameters:
//    pCmdIO - Pointer to the command device node.
//    argc - Number of arguments.
//    argv - Array of arguments.
//
// Returns:
//    None
//
// Remarks:
//    None
// *****************************************************************************
void APP_PROV_CmdProcessing
(
    SYS_CMD_DEVICE_NODE* pCmdIO, 
    int argc, 
    char** argv
)
{
    SYS_CONSOLE_PRINT(TERM_RESET"Command Received : prov\r\n");
    if ( (argc < 2) && (argc > 3))
    {
        SYS_CONSOLE_MESSAGE(TERM_RED"Wrong Usage of Command. Check \"prov help\" command\r\n" TERM_RESET);
        return;
    }
    if ((argc == 3) && ((0 == strcmp(argv[1], "GEN")) || 
                (0 == strcmp(argv[1], "USA")) ||
                (0 == strcmp(argv[1], "EMEA"))))
    {
        argv[strlen(argv[1])] = '\0';
        APP_WINCS02_SetRegDomain ( (const char *)argv[1] );
        
        if (strcmp(argv[2], "start")== 0)
        {
            g_appProvCtx.state = APP_PROV_STATE_SET_REG_DOMAIN;
        }
        else
        {
            SYS_CONSOLE_MESSAGE(TERM_RED"Wrong Command. Check \"prov help\" command\r\n" TERM_RESET);
        }
    }
    else if (strcmp(argv[1], "stop")== 0)
    {
        g_appProvCtx.state = APP_PROV_STATE_STOP;
    }
    else if (strcmp(argv[1], "help")== 0)
    {
        APP_PROV_CmdPrintUsage();
    }
    else
    {
        SYS_CONSOLE_PRINT(TERM_RED "Invalid Command parameter <REGDOMAIN>. Check \"prov help\" command\r\n\r\n" TERM_RESET);
    }
    return;
}

// *****************************************************************************
// *****************************************************************************
// Command Table for Provisioning
//
// Summary:
//    Defines the command table for the provisioning application.
//
// Description:
//    This table defines the commands that can be processed by the provisioning
//    application.
//
// Remarks:
//    None
// *****************************************************************************
static const SYS_CMD_DESCRIPTOR PROVCmdTbl[] =
{
    {"prov", APP_PROV_CmdProcessing, ": Provision commands processing"},
};

// *****************************************************************************
/**
 * @brief Callback handler for WiFi provisioning events.
 *
 * This function is called whenever a WiFi provisioning event occurs. It handles
 * the event based on the type of event received and the provisioning handle.
 *
 * @param event The WiFi provisioning event that occurred. This is of type
 *              SYS_WINCS_PROV_EVENT_t and indicates the specific event.
 * @param provHandle The handle associated with the provisioning event. This is
 *                   of type SYS_WINCS_PROV_HANDLE_t and is used to identify
 *                   the specific provisioning instance.
 */
// *****************************************************************************

static void SYS_WINCS_WIFIPROV_CallbackHandler 
( 
    SYS_WINCS_PROV_EVENT_t event, 
    SYS_WINCS_PROV_HANDLE_t provHandle
)
{
    switch(event)
    {
        /**<Provisionging complete*/
        case SYS_WINCS_PROV_COMPLETE:
        {
            SYS_WINCS_PROV_SrvCtrl(SYS_WINCS_PROV_DISABLE, NULL);
//            SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_SET_CALLBACK, SYS_WINCS_WIFI_CallbackHandler);
            
            // Connect to the received AP configurations
            SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_SET_PARAMS, (SYS_WINCS_WIFI_HANDLE_t)provHandle);
            
            //If autoConnect is false 
            //SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_STA_CONNECT, NULL);
            break;
        }    
        
        /**<Provisionging Failure*/
        case SYS_WINCS_PROV_FAILURE:
        {
            break;
        }
        
        default:
        {
            break;
        }
    }
    
}




void APP_PROV_CmdInitialize(void) 
{
    //$TODO : APP_PROC_CMD_Initiakize() take these 3 lines inside this 
    if (!SYS_CMD_ADDGRP(PROVCmdTbl, sizeof(PROVCmdTbl) / sizeof(*PROVCmdTbl), "prov", ": Provision commands"))
    {
        SYS_ERROR(SYS_ERROR_ERROR, "Failed to create PROV Commands\r\n");
    }
}

// *****************************************************************************
// *****************************************************************************
// Function: APP_PROV_Initialize
//
// Summary:
//    Initializes the provisioning application.
//
// Description:
//    This function initializes the provisioning application, registers the provisioning
//    command group, and sets the initial state of the provisioning application.
//
// Parameters:
//    None
//
// Returns:
//    None
//
// Remarks:
//    None
// *****************************************************************************
void APP_PROV_Initialize(void) 
{
    APP_PROV_CmdInitialize();
    
    g_appProvCtx.state = APP_PROV_STATE_INIT;
}

// *****************************************************************************
// *****************************************************************************
// Function: APP_PROV_Tasks
//
// Summary:
//    Manages the provisioning application tasks.
//
// Description:
//    This function defines the state machine for the provisioning application and
//    manages the tasks based on the current state.
//
// Parameters:
//    None
//
// Returns:
//    None
//
// Remarks:
//    None
// *****************************************************************************
void APP_PROV_Tasks(void)
{
    switch (g_appProvCtx.state)
    {
        case APP_PROV_STATE_INIT:
        {
            g_appProvCtx.state = APP_PROV_STATE_SERVICE_TASKS;
            break;
        }

        case APP_PROV_STATE_SET_REG_DOMAIN:
        {
            
            // Set the callback handler for Wi-Fi events
            g_appProvCtx.state = APP_PROV_STATE_WAIT;
            
            SYS_CONSOLE_PRINT(TERM_YELLOW"[APP_PROV] : Setting REG domain to " TERM_UL "%s\r\n"TERM_RESET ,APP_WINCS02_GetRegDomain());
            // Set the regulatory domain
            if (SYS_WINCS_FAIL == SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_SET_REG_DOMAIN, APP_WINCS02_GetRegDomain()))
            {
                g_appProvCtx.state = APP_STATE_WINCS_ERROR;
                break;
            }
            break;
        }
        
        case APP_PROV_STATE_PROV_ENABLE:
        {
            
            SYS_WINCS_PROV_SrvCtrl(SYS_WINCS_PROV_SET_CALLBACK, (void *)SYS_WINCS_WIFIPROV_CallbackHandler);

            if (SYS_WINCS_FAIL == SYS_WINCS_PROV_SrvCtrl(SYS_WINCS_PROV_ENABLE, NULL))
            {
                g_appProvCtx.state = APP_PROV_STATE_ERROR;
                break;
            }
            g_appProvCtx.state = APP_PROV_STATE_SERVICE_TASKS;
            break;
        }

        case APP_PROV_STATE_ERROR:
        {
            SYS_CONSOLE_PRINT(TERM_RED "[APP_ERROR] : ERROR in Provision task" TERM_RESET);
            g_appProvCtx.state = APP_PROV_STATE_SERVICE_TASKS;
            break;
        }

        case APP_PROV_STATE_WAIT:
        {
            if (true == APP_WINCS02_GetRegDomainStatus())
            {
                g_appProvCtx.state = APP_PROV_STATE_PROV_ENABLE;
            }
            break;
        }
        
        case APP_PROV_STATE_SERVICE_TASKS:
        {
            break;
        }

        case APP_PROV_STATE_STOP:
        {
            SYS_CONSOLE_PRINT(TERM_RED "[APP_PROV] : Provision service Stopped\r\n " TERM_RESET);
            // Unregister application callbacks 
            SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_SET_SRVC_CALLBACK, NULL);

            SYS_WINCS_WIFI_SrvCtrl(SYS_WINCS_WIFI_AP_DISABLE, NULL);
            
            // Unregister network callbacks and disable DHCP server
            SYS_WINCS_NET_SockSrvCtrl(SYS_WINCS_NET_SOCK_SET_SRVC_CALLBACK, NULL);

            if (SYS_WINCS_FAIL == SYS_WINCS_NET_SockSrvCtrl(SYS_WINCS_NET_DHCP_SERVER_DISABLE, NULL))
            {
                g_appProvCtx.state = APP_PROV_STATE_ERROR;
                break;
            }
            // Stop AP disable 
            g_appProvCtx.state = APP_PROV_STATE_SERVICE_TASKS;
            break;
        }

        case APP_PROV_STATE_DONE:
        {
            break;
        }
    }
}