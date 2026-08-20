/* ----------------------------------------------------------------------------
  Open Source Replica CDI 'OSR-CDI' system for YAMAHA 2T motorcycle
  ----------------------------------------------------------------------------
Copyright (c) 2013-2024, Rilassaru (http://rilassaru.blog.jp/)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met: 

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer. 
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies, 
either expressed or implied, of the FreeBSD Project.
----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
  This program has been based on framework of the Microchip Inc.
  Microchip owns the copyright of the implementation part of the 
  USB driver, and HID protocol.Please refer to the header part of
  each source code for details.
  (modified from (this file name).c included in MCHPFSUSB v1.2/v1.3)
  --------------------------------------------------------------------------*/
#include "usb.h"

#ifndef __XC8__
#pragma romdata
#endif

/* Device Descriptor */
ROM USB_DEV_DSC device_dsc ={
    sizeof (USB_DEV_DSC), // Size of this descriptor in bytes
    DSC_DEV, // DEVICE descriptor type
    0x0200, // USB Spec Release Number in BCD format
    0x00, // Class Code
    0x00, // Subclass code
    0x00, // Protocol code
    EP0_BUFF_SIZE, // Max packet size for EP0, see usb_config.h
    0x04D8, // Vendor ID: Microchip
    0x003C, // Product ID: HID Bootloader
    0x0101, // Device release number in BCD format
    0x01, // Manufacturer string index
    0x02, // Product string index
    0x00, // Device serial number string index
    0x01 // Number of possible configurations
};

/* Configuration 1 Descriptors */
ROM uint8_t CFG01[CONFIG_DESC_TOTAL_LEN] = {
    /* Configuration Descriptor */
    sizeof (USB_CFG_DSC), // Size of this descriptor in bytes
    DSC_CFG, // CONFIGURATION descriptor type
    (uint8_t) CONFIG_DESC_TOTAL_LEN, // Total length of data for this cfg - LSB
    (uint8_t) (CONFIG_DESC_TOTAL_LEN >> 8), // Total length of data for this cfg - MSB
    1, // Number of interfaces in this cfg
    1, // Index value of this configuration
    0, // Configuration string index
    _DEFAULT, // Attributes, see usb_device.h
    50, // Max power consumption (2X mA)

    /* Interface Descriptor */
    sizeof (USB_INTF_DSC), // Size of this descriptor in bytes
    DSC_INTF, // INTERFACE descriptor type
    0, // Interface Number
    0, // Alternate Setting Number
    2, // Number of endpoints in this intf
    HID_INTF, // Class code
    0, // Subclass code, no subclass
    0, // Protocol code, no protocol
    0, // Interface string index

    /* HID Class-Specific Descriptor */
    sizeof (USB_HID_DSC), // Size of this descriptor in bytes
    DSC_HID, // HID descriptor type
    0x11, // HID Spec Release Number in BCD format (0x0111 = v1.11) - LSB
    0x01, // HID Spec Release Number in BCD format (0x0111 = v1.11) - MSB
    0x00, // Country Code (0x00 for Not supported)
    HID_NUM_OF_DSC, // Number of class descriptors, see usb_config.h
    DSC_RPT, // Report descriptor type
    (uint8_t) HID_RPT01_SIZE, // Size of the report descriptor - LSB
    (uint8_t) ((uint16_t) HID_RPT01_SIZE >> 8), // Size of the report descriptor - MSB

    /* Endpoint Descriptor */
    sizeof (USB_EP_DSC), //Endpoint descriptor size
    DSC_EP, //Type of descriptor (endpoint)
    _EP01_IN, //Endpoint number + direction
    _INT, //Endpoint transfer type implemented
    HID_INT_IN_EP_SIZE, //LSB - endpoint size
    0x00, //MSB - endpoint size
    0x01, //bInterval

    /* Endpoint Descriptor */
    sizeof (USB_EP_DSC), //Endpoint descriptor size
    DSC_EP, //Type of descriptor (endpoint)
    _EP01_OUT, //Endpoint number + direction
    _INT, //Endpoint transfer type implemented
    HID_INT_OUT_EP_SIZE, //LSB - endpoint size
    0x00, //MSB - endpoint size
    0x01 //bInterval
};

ROM struct {
    uint8_t bLength;
    uint8_t bDscType;
    uint16_t string[1];
} sd000 = {
    sizeof (sd000), DSC_STR, 0x0409
};

ROM struct {
    uint8_t bLength;
    uint8_t bDscType;
    uint16_t string[25];
} sd001 = {
    sizeof (sd001), DSC_STR,
//    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18   19   20   21   22   23   24   25
    'O', 'p', 'e', 'n', ' ', 'S', 'o', 'u', 'r', 'c', 'e', ' ', 'w', '/', ' ', 'R', 'i', 'l', 'a', 's', 's', 'a', 'r', 'u', '.'
//  'Y', 'A', 'M', 'A', 'H', 'A', '-', '2', 'T', '-', 'D', 'e', 'n', 's', 'o', 'T', 'o', 'm', 'o', 'n', 'o', 'K', 'a', 'i', '.'
};

ROM struct {
    uint8_t bLength;
    uint8_t bDscType;
    uint16_t string[26];
} sd002 = {
    sizeof (sd002), DSC_STR,
  // 0    1    2    3    4    5    6    7    8    9    0    1    2    3    4    5    6    7    8    9    0    1    2    3    4    5
    'O', 'S', 'R', '-', 'C', 'D', 'I', ' ', 'S', 'Y', 'S', 'T', 'E', 'M', ' ', 'V', 'e', 'r', '.', '1', '.', '5', '.', '6', 'a', '\0'
};

ROM uint8_t hid_rpt01[HID_RPT01_SIZE] =
        //  First byte in each row is the "item".  First byte's two least significant
        //  bits are the number of data bytes that follow, but encoded (0=0, 1=1, 2=2, 3=4 bytes).
        //  bSize should match number of bytes that follow, or REPORT descriptor parser won't work.  The bytes
        //  that follow in each item line are data bytes
{
    0x06, 0x00, 0xFF, // Usage Page = 0xFF00 (Vendor Defined Page 1)
    0x09, 0x01, // Usage (Vendor Usage 1)
    0xA1, 0x01, // Collection (Application)
    0x19, 0x01, //      Usage Minimum
    0x29, 0x40, //      Usage Maximum   //64 input usages total (0x01 to 0x40)
    0x15, 0x00, //      Logical Minimum (data bytes in the report may have minimum value = 0x00)
    0x26, 0xFF, 0x00, //Logical Maximum (data bytes in the report may have maximum value = 0x00FF = unsigned 255)
    0x75, 0x08, //      Report Size: 8-bit field size
    0x95, 0x40, //      Report Count: Make sixty-four 8-bit fields (the next time the parser hits an "Input", "Output", or "Feature" item)
    0x81, 0x00, //      Input (Data, Array, Abs): Instantiates input packet fields based on the above report size, count, logical min/max, and usage.
    0x19, 0x01, //      Usage Minimum
    0x29, 0x40, //      Usage Maximum   //64 output usages total (0x01 to 0x40)
    0x91, 0x00, //      Output (Data, Array, Abs): Instantiates output packet fields.  Uses same report size and count as "Input" fields, since nothing new/different was specified to the parser since the "Input" item.
    0xC0 // End Collection
};


ROM unsigned char* ROM USB_SD_Ptr[] ={
    (ROM const unsigned char *ROM) & sd000,
    (ROM const unsigned char *ROM) & sd001,
    (ROM const unsigned char *ROM) & sd002
};


#ifndef __XC8__
#pragma code
#endif

/** EOF usb_descriptors.c ****************************************************/
