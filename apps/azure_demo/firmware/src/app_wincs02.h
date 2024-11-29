/*******************************************************************************
  MPLAB Harmony Application Source header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_wincs02.h

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
    files********************************************************************** */

#ifndef _EXAMPLE_FILE_NAME_H    /* Guard against multiple inclusion */
#define _EXAMPLE_FILE_NAME_H

#define TERM_GREEN "\x1B[32m"
#define TERM_RED   "\x1B[31m"
#define TERM_YELLOW "\x1B[33m"
#define TERM_CYAN "\x1B[36m"
#define TERM_WHITE "\x1B[47m"
#define TERM_RESET "\x1B[0m"
#define TERM_BG_RED "\x1B[41m" 
#define TERM_BOLD "\x1B[1m" 
#define TERM_UL "\x1B[4m"

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

/* This section lists the other files that are included in this file.
 */

/* TODO:  Include other files here if needed. */


/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif


/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Constants                                                         */
/* ************************************************************************** */
/* ************************************************************************** */

/*  A brief description of a section can be given directly below the section
    banner.
 */
    
#define AZURE_PUB_PROPERTY      "$iothub/twin/PATCH/properties/reported/?$rid=1"
#define AZURE_PUB_TWIN_GET      "$iothub/twin/GET/?$rid=getTwin"
#define AZURE_PUB_CMD_RESP      "$iothub/methods/res/200/?$%s" 

#define AZURE_MSG_IPADDRESS     "\\\"ipAddress\\\":\\\"%s\\\""

/* Properties and Telemetry reporting format */
#define AZURE_FMT_BUTTON_TEL    "{\\\"buttonEvent\\\": {\\\"button_name\\\":\\\"SW0\\\", \\\"press_count\\\":%lu}"
#define AZURE_FMT_COUNTER_TEL   "{\\\"counter\\\": \\\"%lu\\\"}"
#define AZURE_FMT_ECHO_RSP      "{\\\"echoString\\\":\\\"%s\\\"}"

/* Parsing Tags for the Azure messages */
#define AZURE_ECHO_TAG          "\"echoString\\\":\\\""
/* System Tick timer tick for 1Sec */
#define APP_SYS_TICK_COUNT_1SEC 1000
    
/* APP Cloud Telemetry Rate in seconds */
#define APP_CLOUD_REPORT_INTERVAL  3 * APP_SYS_TICK_COUNT_1SEC
 
#define APP_BUILD_HASH_SZ 5
#define APP_DI_IMAGE_INFO_NUM 2
    
#define AZURE_PUB_TWIN_GET      "$iothub/twin/GET/?$rid=getTwin"
    
#define CLOUD_SUBACK_HANDLER()        APP_WINCS_azureSubAckHandler()
#define CLOUD_SUBMSG_HANDLER(msg)     APP_WINCS_AzureSubMsgHandler(msg)
#define CLOUD_STATE_MACHINE()         APP_WINCS_AzureTask()
    

    typedef struct
{
    /* Version number structure. */
    struct
    {
        /* Major version number. */
        uint16_t major;

        /* Major minor number. */
        uint16_t minor;

        /* Major patch number. */
        uint16_t patch;
    } version;
} APP_DRIVER_VERSION_INFO;

typedef struct
{
    /* Flag indicating if this information is valid. */
    bool isValid;

    /* Version number structure. */
    struct
    {
        /* Major version number. */
        uint16_t major;

        /* Major minor number. */
        uint16_t minor;

        /* Major patch number. */
        uint16_t patch;
    } version;

    /* Build date/time structure. */
    struct
    {
        uint8_t hash[APP_BUILD_HASH_SZ];

        uint32_t timeUTC;
    } build;
} APP_FIRMWARE_VERSION_INFO;


// *****************************************************************************
/*  Device Information

  Summary:
    Defines the device information.

  Description:
    This data type defines the device information of the WINC.

  Remarks:
    None.
*/

typedef struct
{
    /* Flag indicating if this information is valid. */
    bool isValid;

    /* WINC device ID. */
    uint32_t id;

    /* Number of flash images. */
    uint8_t numImages;

    /* Information for each device image. */
    struct
    {
        /* Sequence number. */
        uint32_t seqNum;

        /* Version information. */
        uint32_t version;

        /* Source address. */
        uint32_t srcAddr;
    } image[APP_DI_IMAGE_INFO_NUM];
} APP_DEVICE_INFO;


// *****************************************************************************
/* Application states

  Summary:
    Application states enumeration

  Description:
    This enumeration defines the valid application states.  These states
    determine the behavior of the application at various times.
*/


typedef enum
{
    // State to print Message 
    APP_STATE_WINCS_PRINT = -1,
            
    // Initial state of the application
    APP_STATE_WINCS_INIT = 0,

    // State to open the Wi-Fi driver
    APP_STATE_WINCS_OPEN_DRIVER,

    // State to retrieve device information
    APP_STATE_WINCS_DEVICE_INFO,

    // State to set the regulatory domain for Wi-Fi
    APP_STATE_WINCS_SET_REG_DOMAIN,

    // State to print certs and keys on device 
    APP_STATE_WINCS_PRINT_CERTS_KEYS,
            
    // State to configure Wi-Fi parameters
    APP_STATE_WINCS_SET_WIFI_PARAMS,
    
    // State to connect to azure cloud
    APP_STATE_WINCS_CONNECT_AZURE,
            
    // State to handle errors
    APP_STATE_WINCS_ERROR,

    // State to perform regular service tasks and wait for callback
    APP_STATE_WINCS_SERVICE_TASKS

    /* TODO: Define states used by the application state machine. */
            
} APP_WINCS02_STATES;

typedef struct
{
    /* The application's current state */
    APP_WINCS02_STATES state;

    /* TODO: Define any additional data used by the application. */

} APP_WINCS02_DATA;

/*Shows the he application's state*/
typedef enum {
    /*InitializaTION State*/
    APP_SYS_INIT,
            
    /*Cloud up state*/
    APP_CLOUD_UP
} APP_STATE_t;

void APP_WINCS02_Initialize ( void );
void APP_WINCS02_Tasks ( void );


    /* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* _EXAMPLE_FILE_NAME_H */

/* *****************************************************************************
 End of File
 */
