# TCP Client Application

This application demonstrates a traditional open TCP client in the Host Companion mode.<br /> In this application the WINCS02 will host a TCP Client in Wi-Fi STA mode. The user is<br /> required to configure the Wi-Fi credentials for Home-AP and provide the necessary<br /> details for the remote TCP server configuration. By default the application will connect<br /> to Home-AP and begin a TCP client socket connection with a TCP server. After<br /> establishing a successful TCP server-client connection, the data exchange will take<br /> place.

# Building the  Application

To build this application, open the project file<br /> \(`apps\tcp_client\firmware\tcp_client_sam_e54_xpro_wincs02.X/`\) in MPLAB X<br /> IDE. For more details on opening the project file in MPLAB X IDE, refer to the [Opening the Project File](https://onlinedocs.microchip.com/oxy/GUID-92FEB3A1-C10F-47DF-BF88-C06521800526-en-US-1/GUID-671CCA8C-64AE-4EA1-B144-D46A6FEE76FF.html#GUID-671CCA8C-64AE-4EA1-B144-D46A6FEE76FF__SECTION_YRV_2WM_QYB). The following table<br /> provides details on the project file.

<br />

|Project Name|Description|
|------------|-----------|
|`tcp_client_sam_e54_xpro_wincs02.X`|<br /> -   MPLAB X project for SAM E54 Xplained Pro evaluation kit and WINCS02 Add On Board
-   This application demonstrates a traditional open TCP client in Host Companion mode. After establishing a successful TCP server-client connection, the data exchange will take place

<br />|

<br />

**Running the Application**

1.  Mount the WINCS02 Add On Board on SAM E54 Xplained Pro evaluation kit at respective header. For more details about the boards placement in the SAM E54 X-plained host board, see [Figure 6-70](https://onlinedocs.microchip.com/oxy/GUID-92FEB3A1-C10F-47DF-BF88-C06521800526-en-US-1/GUID-7BA99DE1-89EB-4DD7-973B-974B175D657A.html#GUID-7BA99DE1-89EB-4DD7-973B-974B175D657A__FIG_PKK_14T_11C)
2.  Connect the debugger USB port on the SAM E54 Xplained Pro evaluation kit to computer using a micro-USB cable.
3.  Open the project and launch MCC Harmony3.
4.  Configure Home-AP credentials for STA mode, using the RNWF02 Wi-Fi configuration options. For more details about the Wi-Fi settings configuration, See [Figure 3-64](https://onlinedocs.microchip.com/oxy/GUID-92FEB3A1-C10F-47DF-BF88-C06521800526-en-US-1/GUID-CE9CEDFD-5FD4-4BC4-AB96-17647C430816.html#GUID-CE9CEDFD-5FD4-4BC4-AB96-17647C430816__GUID-98F61951-56D2-4B91-B509-2A796802408B).
5.  Configure TCP Server details such as TCP server IP address and port. For more details about the Net Sock settings configuration, See [Figure 3-74](https://onlinedocs.microchip.com/oxy/GUID-92FEB3A1-C10F-47DF-BF88-C06521800526-en-US-1/GUID-834E84DC-609A-4A37-853F-3552166E1009.html#GUID-834E84DC-609A-4A37-853F-3552166E1009__FIG_TYM_M13_B1C).
6.  Generate the code as illustrated below.

    <br />

    ![](images/GUID-0227005F-6A58-4883-9FB1-395F49202CA6-low.png "Generating the Code")

    <br />

7.  Build and program the code to the hardware using MPLABX IDE.

    <br />

    ![](images/GUID-95D92FA6-7B8F-4C90-A4D2-4B425AEE63CB-low.png "Programming the Board")

    <br />

8.  Open the Terminal application \(for example, Tera Term or PuTTY\) on the PC
9.  Connect to the â€œEDBG Virtual COM Portâ€? and configure the serial settings as follows:
    1.  Baud: 115200

    2.  Data: 8 Bits

    3.  Parity: None

    4.  Stop: 1 Bit

    5.  Flow Control: None

10. As the board boots up, it will connect to Home-AP and print the IP address obtained. The board will establish a connection with configured TCP Server and print a success message. Once the TCP client-server connection is successful, the application will continue listening on the socket for incoming messages and then write them back to the server.

    <br />

    ![](images/GUID-DA6DC60F-52D4-4D60-AC4B-DB80994EECB3-low.png "TCP Client - Serial Logs")

    <br />


