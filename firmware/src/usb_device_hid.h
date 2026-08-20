/* ----------------------------------------------------------------------------
  Open Source Replica CDI 'OSR-CDI' system for YAMAHA 2T motorcycle
  ----------------------------------------------------------------------------
Copyright(c) 2013-2025, Rilassaru(http://rilassaru.blog.jp/)
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
#ifndef HID_H
#define HID_H

#include "typedefs.h"

// Class-Specific Requests
#define GET_REPORT      0x01
#define GET_IDLE        0x02
#define GET_PROTOCOL    0x03
#define SET_REPORT      0x09
#define SET_IDLE        0x0A
#define SET_PROTOCOL    0x0B

// Class Descriptor Types
#define DSC_HID         0x21
#define DSC_RPT         0x22
#define DSC_PHY         0x23

// Protocol Selection
#define BOOT_PROTOCOL   0x00
#define RPT_PROTOCOL    0x01


// HID Interface Class Code
#define HID_INTF                    0x03

// HID Interface Class SubClass Codes
#define BOOT_INTF_SUBCLASS          0x01

// HID Interface Class Protocol Codes
#define HID_PROTOCOL_NONE           0x00
#define HID_PROTOCOL_KEYBOAD        0x01
#define HID_PROTOCOL_MOUSE          0x02

#define mHIDRxIsBusy()              HID_BD_OUT.Stat.UOWN
#define mHIDTxIsBusy()              HID_BD_IN.Stat.UOWN
#define mHIDGetRptRxLength()        hid_rpt_rx_len


// HID macros
#define mUSBGetHIDDscAdr(ptr)               \
{                                           \
    if(usb_active_cfg == 1)                 \
        ptr = (ROM uint8_t*)&cfg01.hid_i00a00; \
}

#define mUSBGetHIDRptDscAdr(ptr)            \
{                                           \
    if(usb_active_cfg == 1)                 \
        ptr = (ROM uint8_t*)&hid_rpt01;        \
}

#define mUSBGetHIDRptDscSize(count)         \
{                                           \
    if(usb_active_cfg == 1)                 \
        count = sizeof(hid_rpt01);          \
}



typedef struct _USB_HID_DSC_HEADER
{
    uint8_t bDscType;
    uint16_t wDscLength;
} USB_HID_DSC_HEADER;

typedef struct _USB_HID_DSC
{
    uint8_t bLength;       uint8_t bDscType;      uint16_t bcdHID;
    uint8_t bCountryCode;  uint8_t bNumDsc;
    USB_HID_DSC_HEADER hid_dsc_header[HID_NUM_OF_DSC];
    /*
     * HID_NUM_OF_DSC is defined in usb_config.h
     */
} USB_HID_DSC;

extern uint8_t hid_rpt_rx_len;
extern ROM uint8_t hid_rpt01[HID_RPT01_SIZE];


void hid_init_ep(void);
void usb_check_hid_request(void);
void hid_tx_report(char *buffer, uint8_t len);
uint8_t HIDRxReport(char *buffer, uint8_t len);

#endif //HID_H
