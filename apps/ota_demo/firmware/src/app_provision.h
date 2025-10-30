/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_provision.h

  Summary:
    This file contains the header code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 ************************************************************************** */

#ifndef _APP_PROVISION_H    /* Guard against multiple inclusion */
#define _APP_PROVISION_H
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

/* This section lists the other files that are included in this file. */
#include "configuration.h"
#include "system/console/sys_console.h"
#include "system/net/sys_wincs_net_service.h"
#include "system/wifi/sys_wincs_wifi_service.h"
#include "config/sam_e54_xpro_wincs02/configuration.h"
#include "system/wifiprov/sys_wincs_provision_service.h"
#include "config/sam_e54_xpro_wincs02/system/command/sys_command.h"

// *****************************************************************************
// *****************************************************************************
// Provisioning Application States
//
// Summary:
//    Defines the states for the provisioning application state machine.
//
// Description:
//    This enumeration defines the various states that the provisioning application
//    can be in during its lifecycle.
//
// Remarks:
//    None
// *****************************************************************************
typedef enum
{
    /* Application's state machine's initial state. */
    APP_PROV_STATE_INIT = 0,

    APP_PROV_STATE_SET_REG_DOMAIN,
        
    /* State to enable provisioning. */
    APP_PROV_STATE_PROV_ENABLE,

    /* State indicating an error has occurred. */
    APP_PROV_STATE_ERROR,

    /* State indicating the application has completed its tasks. */
    APP_PROV_STATE_DONE,

    /* State to wait for a certain condition or event. */
    APP_PROV_STATE_WAIT,
            
    APP_PROV_STATE_SERVICE_TASKS,

    /* State to stop the provisioning service. */
    APP_PROV_STATE_STOP        
} APP_PROV_STATES;

// *****************************************************************************
// *****************************************************************************
// Provisioning Application Data
//
// Summary:
//    Defines the data structure for the provisioning application.
//
// Description:
//    This structure defines the data used by the provisioning application, including
//    the current state and any additional data required.
//
// Remarks:
//    None
// *****************************************************************************
typedef struct
{
    /* The application's current state */
    APP_PROV_STATES state;

    /* TODO: Define any additional data used by the application. */

} APP_PROV_CONTEXT;


void APP_PROV_CmdInitialize(void);

APP_PROV_STATES APP_PROV_GetProvStatus( void );
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
void APP_PROV_Initialize(void);

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
void APP_PROV_Tasks(void);

// *****************************************************************************
// *****************************************************************************
// Function: APP_PROV_CMDProcessing
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
void APP_PROV_CmdProcessing(SYS_CMD_DEVICE_NODE* pCmdIO, int argc, char** argv);

#endif // _APP_PROVISION_H
