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
#ifndef _USB_DEVICE_H
#define _USB_DEVICE_H

#include "usb.h"

#define USBIF_FLAG  PIR2bits.USBIF
#define USBIE_BIT   PIE2bits.USBIE


// Standard Request Codes
#define GET_STATUS  0
#define CLR_FEATURE 1
#define SET_FEATURE 3
#define SET_ADR     5
#define GET_DSC     6
#define SET_DSC     7
#define GET_CFG     8
#define SET_CFG     9
#define GET_INTF    10
#define SET_INTF    11
#define SYNCH_FRAME 12

// Standard Feature Selectors
#define DEVICE_REMOTE_WAKEUP    0x01
#define ENDPOINT_HALT           0x00


// UCFG Initialization Parameters
#define _PPBM0      0x00            // Pingpong Buffer Mode 0 - ping pong bufferring disabled
#define _PPBM1      0x01            // Pingpong Buffer Mode 1 - ping pong on EP0 OUT only
#define _PPBM2      0x02            // Pingpong Buffer Mode 2 - ping pong on all endpoints
#define _LS         0x00            // Use Low-Speed USB Mode
#define _FS         0x04            // Use Full-Speed USB Mode
#define _TRINT      0x00            // Use internal transceiver
#define _TREXT      0x08            // Use external transceiver
#define _PUEN       0x10            // Use internal pull-up resistor
#define _OEMON      0x40            // Use SIE output indicator
#define _UTEYE      0x80            // Use Eye-Pattern test

// UEPn Initialization Parameters
#define EP_CTRL     0x06            // Cfg Control pipe for this ep
#define EP_OUT      0x0C            // Cfg OUT only pipe for this ep
#define EP_IN       0x0A            // Cfg IN only pipe for this ep
#define EP_OUT_IN   0x0E            // Cfg both OUT & IN pipes for this ep
#define HSHK_EN     0x10            // Enable handshake packet
                                    // Handshake should be disable for isoch

// USB - PICmicro Endpoint Definitions
#define OUT         0
#define IN          1

#define PIC_EP_NUM_MASK 0b01111000
#define PIC_EP_DIR_MASK 0b00000100

#define EP00_OUT    ((0x00<<3)|(OUT<<2))
#define EP00_IN     ((0x00<<3)|(IN<<2))
#define EP01_OUT    ((0x01<<3)|(OUT<<2))
#define EP01_IN     ((0x01<<3)|(IN<<2))
#define EP02_OUT    ((0x02<<3)|(OUT<<2))
#define EP02_IN     ((0x02<<3)|(IN<<2))
#define EP03_OUT    ((0x03<<3)|(OUT<<2))
#define EP03_IN     ((0x03<<3)|(IN<<2))
#define EP04_OUT    ((0x04<<3)|(OUT<<2))
#define EP04_IN     ((0x04<<3)|(IN<<2))
#define EP05_OUT    ((0x05<<3)|(OUT<<2))
#define EP05_IN     ((0x05<<3)|(IN<<2))
#define EP06_OUT    ((0x06<<3)|(OUT<<2))
#define EP06_IN     ((0x06<<3)|(IN<<2))
#define EP07_OUT    ((0x07<<3)|(OUT<<2))
#define EP07_IN     ((0x07<<3)|(IN<<2))
#define EP08_OUT    ((0x08<<3)|(OUT<<2))
#define EP08_IN     ((0x08<<3)|(IN<<2))
#define EP09_OUT    ((0x09<<3)|(OUT<<2))
#define EP09_IN     ((0x09<<3)|(IN<<2))
#define EP10_OUT    ((0x0A<<3)|(OUT<<2))
#define EP10_IN     ((0x0A<<3)|(IN<<2))
#define EP11_OUT    ((0x0B<<3)|(OUT<<2))
#define EP11_IN     ((0x0B<<3)|(IN<<2))
#define EP12_OUT    ((0x0C<<3)|(OUT<<2))
#define EP12_IN     ((0x0C<<3)|(IN<<2))
#define EP13_OUT    ((0x0D<<3)|(OUT<<2))
#define EP13_IN     ((0x0D<<3)|(IN<<2))
#define EP14_OUT    ((0x0E<<3)|(OUT<<2))
#define EP14_IN     ((0x0E<<3)|(IN<<2))
#define EP15_OUT    ((0x0F<<3)|(OUT<<2))
#define EP15_IN     ((0x0F<<3)|(IN<<2))

#define EP0_OUT_EVEN_BDT_INDEX  0
#define EP0_OUT_ODD_BDT_INDEX   1


// Buffer Descriptor Status Register Initialization Parameters
#define _BSTALL     0x04                //Buffer Stall enable
#define _DTSEN      0x08                //Data Toggle Synch enable
#define _INCDIS     0x10                //Address increment disable
#define _KEN        0x20                //SIE keeps buff descriptors enable
#define _DAT0       0x00                //DATA0 packet expected next
#define _DAT1       0x40                //DATA1 packet expected next
#define _DTSMASK    0x40                //DTS Mask
#define _USIE       0x80                //SIE owns buffer
#define _UCPU       0x00                //CPU owns buffer

// USB Device States - To be used with [byte usb_device_state]
#define DETACHED_STATE          0
#define ATTACHED_STATE          1
#define POWERED_STATE           2
#define DEFAULT_STATE           3
#define ADR_PENDING_STATE       4
#define ADDRESS_STATE           5
#define CONFIGURED_STATE        6

// Memory Types for Control Transfer - used in USB_DEVICE_STATUS
#define _RAM 0
#define _ROM 1

// Descriptor Types
#define DSC_DEV     0x01
#define DSC_CFG     0x02
#define DSC_STR     0x03
#define DSC_INTF    0x04
#define DSC_EP      0x05

// USB Endpoint Definitions
#define _EP01_OUT   0x01
#define _EP01_IN    0x81
#define _EP02_OUT   0x02
#define _EP02_IN    0x82
#define _EP03_OUT   0x03
#define _EP03_IN    0x83
#define _EP04_OUT   0x04
#define _EP04_IN    0x84
#define _EP05_OUT   0x05
#define _EP05_IN    0x85
#define _EP06_OUT   0x06
#define _EP06_IN    0x86
#define _EP07_OUT   0x07
#define _EP07_IN    0x87
#define _EP08_OUT   0x08
#define _EP08_IN    0x88
#define _EP09_OUT   0x09
#define _EP09_IN    0x89
#define _EP10_OUT   0x0A
#define _EP10_IN    0x8A
#define _EP11_OUT   0x0B
#define _EP11_IN    0x8B
#define _EP12_OUT   0x0C
#define _EP12_IN    0x8C
#define _EP13_OUT   0x0D
#define _EP13_IN    0x8D
#define _EP14_OUT   0x0E
#define _EP14_IN    0x8E
#define _EP15_OUT   0x0F
#define _EP15_IN    0x8F

// Configuration Attributes
#define _DEFAULT    0x01<<7         //Default Value (Bit 7 is set)
#define _SELF       0x01<<6         //Self-powered (Supports if set)
#define _RWU        0x01<<5         //Remote Wakeup (Supports if set)

// Endpoint Transfer Type
#define _CTRL       0x00            //Control Transfer
#define _ISO        0x01            //Isochronous Transfer
#define _BULK       0x02            //Bulk Transfer
#define _INT        0x03            //Interrupt Transfer

// Isochronous Endpoint Synchronization Type
#define _NS         0x00<<2         //No Synchronization
#define _AS         0x01<<2         //Asynchronous
#define _AD         0x02<<2         //Adaptive
#define _SY         0x03<<2         //Synchronous

// Isochronous Endpoint Usage Type
#define _DE         0x00<<4         //Data endpoint
#define _FE         0x01<<4         //Feedback endpoint
#define _IE         0x02<<4         //Implicit feedback Data endpoint


typedef union _USB_DEVICE_STATUS
{
    uint8_t _byte;
    struct
    {
        unsigned RemoteWakeup:1;// [0]Disabled [1]Enabled: See usb_device.c,usb9.c
        unsigned ctrl_trf_mem:1;// [0]RAM      [1]ROM
    };
} USB_DEVICE_STATUS;

typedef union _BD_STAT
{
    uint8_t _byte;
    struct{
        unsigned BC8:1;
        unsigned BC9:1;
        unsigned BSTALL:1;              //Buffer Stall Enable
        unsigned DTSEN:1;               //Data Toggle Synch Enable
        unsigned INCDIS:1;              //Address Increment Disable
        unsigned KEN:1;                 //BD Keep Enable
        unsigned DTS:1;                 //Data Toggle Synch Value
        unsigned UOWN:1;                //USB Ownership
    };
    struct{
        unsigned :2;
        unsigned PID0:1;
        unsigned PID1:1;
        unsigned PID2:1;
        unsigned PID3:1;
        unsigned :2;
    };
    struct{
        unsigned :2;
        unsigned PID:4;                 //Packet Identifier
        unsigned :2;
    };
} BD_STAT;                              //Buffer Descriptor Status Register

typedef union _BDT
{
    struct
    {
        BD_STAT Stat;
        uint8_t Cnt;
        uint8_t ADRL;                      //Buffer Address Low
        uint8_t ADRH;                      //Buffer Address High
    };
    struct
    {
        unsigned :8;
        unsigned :8;
        uint8_t* ADR;                      //Buffer Address
    };
} BDT;                                  //Buffer Descriptor Table

typedef union _CTRL_TRF_SETUP
{
    struct
    {
        uint8_t _byte[EP0_BUFF_SIZE];
    };

    struct
    {
        uint8_t bmRequestType;
        uint8_t bRequest;
        uint16_t wValue;
        uint16_t wIndex;
        uint16_t wLength;
    };
    struct
    {
        unsigned :8;
        unsigned :8;
        WORD_VAL W_Value;
        WORD_VAL W_Index;
        WORD_VAL W_Length;
    };
    struct
    {
        unsigned Recipient:5;
        unsigned RequestType:2;
        unsigned DataDir:1;
        unsigned :8;
        uint8_t bFeature;
        unsigned :8;
        unsigned :8;
        unsigned :8;
        unsigned :8;
        unsigned :8;
    };
    struct
    {
        unsigned :8;
        unsigned :8;
        uint8_t bDscIndex;
        uint8_t bDscType;
        uint16_t wLangID;
        unsigned :8;
        unsigned :8;
    };
    struct
    {
        unsigned :8;
        unsigned :8;
        uint8_t bDevADR;
        uint8_t bDevADRH;
        unsigned :8;
        unsigned :8;
        unsigned :8;
        unsigned :8;
    };
    struct
    {
        unsigned :8;
        unsigned :8;
        uint8_t bCfgValue;
        uint8_t bCfgRSD;
        unsigned :8;
        unsigned :8;
        unsigned :8;
        unsigned :8;
    };
    struct
    {
        unsigned :8;
        unsigned :8;
        uint8_t bAltID;
        uint8_t bAltID_H;
        uint8_t bIntfID;
        uint8_t bIntfID_H;
        unsigned :8;
        unsigned :8;
    };
    struct
    {
        unsigned :8;
        unsigned :8;
        unsigned :8;
        unsigned :8;
        uint8_t bEPID;
        uint8_t bEPID_H;
        unsigned :8;
        unsigned :8;
    };
    struct
    {
        unsigned :8;
        unsigned :8;
        unsigned :8;
        unsigned :8;
        unsigned EPNum:4;
        unsigned :3;
        unsigned EPDir:1;
        unsigned :8;
        unsigned :8;
        unsigned :8;
    };

} CTRL_TRF_SETUP;

// CTRL_TRF_DATA:
typedef union _CTRL_TRF_DATA
{
    struct
    {
        uint8_t _byte[EP0_BUFF_SIZE];
    };

    struct
    {
        uint8_t _byte0;
        uint8_t _byte1;
        uint8_t _byte2;
        uint8_t _byte3;
        uint8_t _byte4;
        uint8_t _byte5;
        uint8_t _byte6;
        uint8_t _byte7;
    };
    struct
    {
        uint16_t _word0;
        uint16_t _word1;
        uint16_t _word2;
        uint16_t _word3;
    };

} CTRL_TRF_DATA;




// USB Device Descriptor Structure
typedef struct _USB_DEV_DSC
{
    uint8_t bLength;       uint8_t bDscType;      uint16_t bcdUSB;
    uint8_t bDevCls;       uint8_t bDevSubCls;    uint8_t bDevProtocol;
    uint8_t bMaxPktSize0;  uint16_t idVendor;      uint16_t idProduct;
    uint16_t bcdDevice;     uint8_t iMFR;          uint8_t iProduct;
    uint8_t iSerialNum;    uint8_t bNumCfg;
} USB_DEV_DSC;

// USB Configuration Descriptor Structure
typedef struct _USB_CFG_DSC
{
    uint8_t bLength;       uint8_t bDscType;      uint16_t wTotalLength;
    uint8_t bNumIntf;      uint8_t bCfgValue;     uint8_t iCfg;
    uint8_t bmAttributes;  uint8_t bMaxPower;
} USB_CFG_DSC;

typedef struct _USB_INTF_DSC
{
    uint8_t bLength;       uint8_t bDscType;      uint8_t bIntfNum;
    uint8_t bAltSetting;   uint8_t bNumEPs;       uint8_t bIntfCls;
    uint8_t bIntfSubCls;   uint8_t bIntfProtocol; uint8_t iIntf;
} USB_INTF_DSC;

typedef struct _USB_EP_DSC
{
    uint8_t bLength;       uint8_t bDscType;      uint8_t bEPAdr;
    uint8_t bmAttributes;  uint16_t wMaxPktSize;   uint8_t bInterval;
} USB_EP_DSC;


#define mInitializeUSBDriver()      {UCFG = UCFG_VAL;                       \
                                     usb_device_state = DETACHED_STATE;     \
                                     usb_protocol_reset_handler();}

#define mDisableEP1to7()       UEP1=0x00;UEP2=0x00;UEP3=0x00;\
                                UEP4=0x00;UEP5=0x00;UEP6=0x00;UEP7=0x00;


#define mUSBBufferReady(buffer_dsc)                                         \
{                                                                           \
    buffer_dsc.Stat._byte &= _DTSMASK;          /* Save only DTS bit */     \
    buffer_dsc.Stat.DTS = !buffer_dsc.Stat.DTS; /* Toggle DTS bit    */     \
    buffer_dsc.Stat._byte |= _DTSEN;            /* Configure other settings */ \
    buffer_dsc.Stat._byte |= _USIE;             /* Turn ownership to SIE */ \
}

#define MUID_NULL               0
#define MUID_USB9               1
#define MUID_HID                2
#define MUID_CDC                3


#define WAIT_SETUP          0
#define CTRL_TRF_TX         1
#define CTRL_TRF_RX         2

#define SHORT_PKT_NOT_SENT  0
#define SHORT_PKT_PENDING   1
#define SHORT_PKT_SENT      2

#define SETUP_TOKEN         0b00001101
#define OUT_TOKEN           0b00000001
#define IN_TOKEN            0b00001001

#define HOST_TO_DEV         0
#define DEV_TO_HOST         1

#define STANDARD            0x00
#define CLASS               0x01
#define VENDOR              0x02

#define RCPT_DEV            0
#define RCPT_INTF           1
#define RCPT_EP             2
#define RCPT_OTH            3


extern uint8_t ctrl_trf_session_owner;
extern POINTER pSrc;
extern POINTER pDst;
extern WORD_VAL wCount;
extern uint8_t usb_device_state;
extern USB_DEVICE_STATUS usb_stat;
extern uint8_t usb_active_cfg;
extern uint8_t usb_alt_intf[MAX_NUM_INT];

extern volatile BDT ep0Bo;          //Endpoint #0 BD Out
extern volatile BDT ep0Bi;          //Endpoint #0 BD In
extern volatile BDT ep1Bo;          //Endpoint #1 BD Out
extern volatile BDT ep1Bi;          //Endpoint #1 BD In
extern volatile BDT ep2Bo;          //Endpoint #2 BD Out
extern volatile BDT ep2Bi;          //Endpoint #2 BD In
extern volatile BDT ep3Bo;          //Endpoint #3 BD Out
extern volatile BDT ep3Bi;          //Endpoint #3 BD In
extern volatile BDT ep4Bo;          //Endpoint #4 BD Out
extern volatile BDT ep4Bi;          //Endpoint #4 BD In
extern volatile BDT ep5Bo;          //Endpoint #5 BD Out
extern volatile BDT ep5Bi;          //Endpoint #5 BD In
extern volatile BDT ep6Bo;          //Endpoint #6 BD Out
extern volatile BDT ep6Bi;          //Endpoint #6 BD In
extern volatile BDT ep7Bo;          //Endpoint #7 BD Out
extern volatile BDT ep7Bi;          //Endpoint #7 BD In
extern volatile BDT ep8Bo;          //Endpoint #8 BD Out
extern volatile BDT ep8Bi;          //Endpoint #8 BD In
extern volatile BDT ep9Bo;          //Endpoint #9 BD Out
extern volatile BDT ep9Bi;          //Endpoint #9 BD In
extern volatile BDT ep10Bo;         //Endpoint #10 BD Out
extern volatile BDT ep10Bi;         //Endpoint #10 BD In
extern volatile BDT ep11Bo;         //Endpoint #11 BD Out
extern volatile BDT ep11Bi;         //Endpoint #11 BD In
extern volatile BDT ep12Bo;         //Endpoint #12 BD Out
extern volatile BDT ep12Bi;         //Endpoint #12 BD In
extern volatile BDT ep13Bo;         //Endpoint #13 BD Out
extern volatile BDT ep13Bi;         //Endpoint #13 BD In
extern volatile BDT ep14Bo;         //Endpoint #14 BD Out
extern volatile BDT ep14Bi;         //Endpoint #14 BD In
extern volatile BDT ep15Bo;         //Endpoint #15 BD Out
extern volatile BDT ep15Bi;         //Endpoint #15 BD In

extern CTRL_TRF_SETUP SetupPkt;
volatile extern CTRL_TRF_DATA CtrlTrfData;

extern volatile unsigned char hid_report_out[HID_INT_OUT_EP_SIZE];
extern volatile unsigned char hid_report_in[HID_INT_IN_EP_SIZE];

extern ROM USB_DEV_DSC device_dsc;
extern ROM uint8_t CFG01[CONFIG_DESC_TOTAL_LEN];
extern ROM const unsigned char *ROM USB_CD_Ptr[];
extern ROM unsigned char* ROM USB_SD_Ptr[];

void usb_device_init(void);
void usb_check_bus_status(void);
void usb_soft_attach(void);
void usb_soft_detach(void);
void usb_device_tasks(void);
void usb_disable_with_long_delay(void);
void delay_routine(unsigned int DelayAmount);
#define USBGetDeviceState() usb_device_state
#define USBIsDeviceSuspended()  UCONbits.SUSPND


#endif //_USB_DEVICE_H
