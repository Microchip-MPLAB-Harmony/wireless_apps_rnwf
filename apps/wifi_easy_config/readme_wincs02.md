# Wi-Fi Easy Configuration

This WINCS02 application uses the Wi-Fi ProvisionMode along with Microchip’s Wi-Fi<br /> Provisioning mobile application to explain how user can provision the device with<br /> Home-AP credentials. The device will boot-up in ProvisionMode, user will connect to<br /> ProvisionMode SSID via a third party STA \(Mobile phone\). User will then provide Home-AP<br /> credentials to the device using Microchip’s mobile app - Wi-Fi Provisioning. After the<br /> successful provisioning, device mode is set to Wi-Fi StationMode. It will then connect<br /> to Home-AP and gets an IP address.

## Building the Application

To build this application, open the project file<br /> \(`apps\Wi-Fi_easy_config\firmware\wifi_easy_config_sam_e54_xpro_wincs02.X/`\) in<br /> MPLAB X IDE. For more details on opening the project file in MPLAB X IDE, refer to the<br /> [Opening the Project File](https://onlinedocs.microchip.com/oxy/GUID-92FEB3A1-C10F-47DF-BF88-C06521800526-en-US-1/GUID-671CCA8C-64AE-4EA1-B144-D46A6FEE76FF.html#GUID-671CCA8C-64AE-4EA1-B144-D46A6FEE76FF__SECTION_YRV_2WM_QYB). The following table<br /> provides details on the project file.

<br />

|Project Name|Description|
|------------|-----------|
|`wifi_easy_config_sam_e54_xpro_wincs02.X`|<br /> -   MPLABX project for SAM E54 Xplained Pro evaluation kit and WINCS02 Add On Board
-   This application demonstrates on how user can configure the device with Home-AP credentials

<br />|

<br />

## Running the Application

1.  Mount the WINCS02 Add On Board on SAM E54 Xplained Pro evaluation kit at respective header. For more details about the boards placement in the SAM E54 X-plained host board, see Figure \(Will be linked soon\)
2.  Connect the debugger USB port on the SAM E54 Xplained Pro evaluation kit to computer using a micro-USB cable.
3.  Open the project and launch MCC Harmony3.
4.  Configure the WINCS02 Wi-Fi ProvisionMode credentials, using the WINCS02 Wi-Fi configuration options. For more details about the Wi-Fi settings configuration, See [Wi-Fi Settings: ProvisionMode](GUID-030C3AA1-022D-4416-B6F8-ED75734E8708.md#IMAGE_WJK_WKV_QBC)
5.  Generate the code as illustrated below

    <br />

    ![](docs/images/GUID-0227005F-6A58-4883-9FB1-395F49202CA6-low.png "Generating the Code")

    <br />

6.  Build and program the code to the hardware using MPLABX IDE

    <br />

    ![](docs/images/GUID-95D92FA6-7B8F-4C90-A4D2-4B425AEE63CB-low.png "Programming the Board")

    <br />

7.  Open the Terminal application \(for example, Tera Term or PuTTY\) on the PC'Connect to the “EDBG Virtual COM Port�? and configure the serial settings as follows:
    1.  **Baud:** 115200
    2.  **Data:** 8 Bits
    3.  **Parity:** None
    4.  **Stop:** 1 Bit
    5.  **Flow Control:** None
8.  The board boots up in Wi-Fi ProvisionMode, as per the configurations given in step 4. ProvisionMode SSID, IP address and Server port number will be available on the serial console as illustrated in the following figure. Connect to the ProvisionMode SSID via third party STA \(any Mobile Phone\).
    <br />

    ![](docs/images/GUID-FC1B33B7-FD23-4189-A89B-F10C6967B560-low.png "ProvisionMode Serial Logs")

    <br />
9.  After establishing a successful connection between mobile phone and the ProvisionMode SSID, open Wi-Fi Provisioning application in mobile phone and enter Server IP address and port number as illustrated below and press Connect.

    

10. On the next page, select AP and provide the Password and Press Send

    <br />

    ![](docs/images/GUID-17C9E8AC-690D-42C7-8E8C-C337B441A527-low.png "Connecting to Provisioning Page")

    ![](docs/images/GUID-1BF7C696-1C3A-4917-BB90-2A1DD878102A-low.png)

    <br />

11. On the next page, select AP and provide the Password and Press Send

    ![](docs/images/GUID-11BECFAC-CE96-446D-A7F8-8EECAC86580B-low.png)

12. After successful provisioning, the board will connect to the Home-AP and gets an IP address. This IP address is displayed on the serial console as illustrated below.
    1.  STA Mode Serial Logs

        ![](docs/images/GUID-FC1B33B7-FD23-4189-A89B-F10C6967B560-low.png "ProvisionMode Serial Logs")



