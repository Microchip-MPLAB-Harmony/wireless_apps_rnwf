---
parent: Harmony 3 Wi-Fi SoftAP application examples for SAM9X7 family
title: softAP 
has_children: false
has_toc: false
---

[![MCHP](https://www.microchip.com/ResourcePackages/Microchip/assets/dist/images/logo.png)](https://www.microchip.com)

# Wi-Fi SoftAP Application 

This application enable softAP. User can connect computer and mobile phone to the softAP and access a static web page through a browser.

## Downloading and building the application

To clone or download this application from Github, go to the [main page of this repository](https://github.com/Microchip-MPLAB-Harmony/wireless_apps_rnwf) and then click Clone button to clone this.

Path of the application within the repository is **apps/wifi_softap/firmware** .

To build the application, refer to the following table and open the project using its IDE.

| Project Name      | Description                                    |
| ----------------- | ---------------------------------------------- |
| wifi_ap_sam9x75_curiosity_wincs02.X | MPLABX project for [SAM9X75 CURIOSITY DEVELOPMENT BOARD](https://www.microchip.com/en-us/development-tool/EV31H43A) |
|||

## Setting up AT91Bootstrap loader

To load the application binary onto the target device, we need to use at91bootstrap loader. Refer to the [at91bootstrap loader documentation](https://onlinedocs.microchip.com/oxy/GUID-270C6E29-0B84-4557-9BB9-8CF21183FAD7-en-US-2/GUID-0FA935BA-EE65-4BB4-B09E-A70E91D1EC02.html) for details on how to configure, build and run bootstrap loader project and use it to bootstrap the application binaries.

## Setting up the hardware

The following table shows the target hardware for the application projects.

| Project Name| Board|
|:---------|:---------:|
| iperf_sam_9x75_curiosity_wincs02.X | [SAM9X75 CURIOSITY DEVELOPMENT BOARD](https://www.microchip.com/en-us/development-tool/EV31H43A) |
|||

### Setting up [SAM9X75 CURIOSITY DEVELOPMENT BOARD](https://www.microchip.com/en-us/development-tool/EV31H43A) and perform Device Firmware Upgrade (DFU)

When you use the WIxCS02 M.2 Wi-Fi Module the 1st time, you need to perform DFU to upgrade the appropriate firmware in the M.2 Wi-Fi Module.  
If the M.2 Wi-Fi module was performed the DFU, skip step 1 and step 5.

1. Program the WIxCS02 M.2 Wi-Fi Module with the correct firmware image

    1.1 Download the Harmony_support.bin and boot.bin from the repository, GitHub [EA03X56A-wifi/m2](https://github.com/MicrochipTech/GSG/tree/main/EA03X56A/wifi/m2/)

    **Notes:**
    The file named "boot.bin" is used both in the programming of the M.2 module and during the application runtime.The "boot.bin" for programming the M.2 module's Firmware can be found at GitHub [EA03X56A-wifi/m2](https://github.com/MicrochipTech/GSG/tree/main/EA03X56A/wifi/m2/) while the "boot.bin" for application runtime can be found under deps folder at the root of the repository. 
    These "boot.bin" files are two distinct files that share the same name. Ensure that the copied files is taken from the specific Github link for that step.

    1.2 Copy the downloaded Harmony_support.bin and boot.bin to the SD card.  

2. Connect the components as shown in the accompanying image:

    2.1 Connect WIxCS02 M.2 Wi-Fi Module to J20 M.2 connector.

    2.2 SD card inserted on the SD Boot J14 slot.

    2.3 Serial-USB dongle to UART DEBUG J35 header.

    2.4 Set J24:M.2  I/F SEL to SPI,
    
    2.5 USB-A to Micro ​​​​​USB-B cable to J2 Micro ​​​​USB-B port for power.

    <p align="center"><img src="docs/images/wireless-kit-wifi-h3.png">
    </p>

3. Open the Terminal application \(Ex.:Tera term\) on the computer  
   Connect to the "USB-FTDI" COM port and configure the serial settings as follows:

    -   Baud : 115200

    -   Data : 8 Bits

    -   Parity : None

    -   Stop : 1 Bit

    -   Flow Control : None

4. Power on the board
5. Confirm using the console that the image copied in Step 1.2 is used in the Device Firmware Update. 
    <p align="center"><img src="docs/images/dfu_console_log.png">
    </p>
6. When you run the applicatin in the next section, unplug the SD card if you use the JTAG debugger. Or replace the boot.bin (which found under deps folder at the root of the repository) in the SD card if SD card is used to boot the applicaiton. Refer to to above section *Setting up AT91Bootstrap loader* for more details.

## Running the Application


1. Build and program the generated code into the hardware using MPLABX IDE
2. Open the Terminal application \(Ex.:Tera term\) on the computer  
   Connect to the "USB-FTDI" COM port and configure the serial settings as follows:

    -   Baud : 115200

    -   Data : 8 Bits

    -   Parity : None

    -   Stop : 1 Bit

    -   Flow Control : None

3. When the board is booting up, it enable softAP (SSID:microchip-SoftAP, password: password) and show below log:
    <p align="center"><img src="docs/images/boot_up_log.png">
    </p>

4. On the PC, connect to the access point using the network name "microchip-SoftAP".
    <p align="center"><img src="docs/images/pc_connect_ap.png">
    </p>

5. Enter "password" for the passphrase/password 

6. On the PC open a browser then connect to the access point using the address: http://192.168.1.1 
    <p align="center"><img src="docs/images/browser_access_webpage.png">
    </p>

7. Confirm on the browser the page shown below is displayed.
    <p align="center"><img src="docs/images/webpage.png">
    </p>

