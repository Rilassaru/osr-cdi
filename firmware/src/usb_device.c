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
 
 #include "usb.h"
#include "HardwareProfile.h"

void usb_check_std_request(void);
void usb_suspend(void);
void usb_protocol_reset_handler(void);
void usb_wake_from_suspend(void);
void usb_std_get_dsc_handler(void);
void usb_std_set_cfg_handler(void);
void usb_std_get_status_handler(void);
void usb_std_feature_req_handler(void);
void usb_ctrl_trf_setup_handler(void);
void usb_ctrl_trf_in_handler(void);
void usb_ctrl_trf_tx_service(void);
void usb_ctrl_ep_service_complete(void);
void load_bdt_and_set_uown(uint8_t BDTIndexToLoad);
#if !defined(ENABLE_CONTROL_TRANSFERS_WITH_OUT_DATA_STAGE)
#define usb_ctrl_trf_out_handler(a)
#endif



#ifndef __XC8__
#pragma udata
#endif
uint8_t bTRNIFCount;
uint8_t ctrl_trf_state;             // Control Transfer State
uint8_t ctrl_trf_session_owner;     // Current transfer session owner
POINTER pSrc;                       // Data source pointer
POINTER pDst;                       // Data destination pointer
WORD_VAL wCount;                    // Data counter
uint8_t short_pkt_status;           // Flag used by Control Transfer Read
CTRL_TRF_SETUP SetupPkt;
bool EP0OutOddNeedsArmingNext;
BDT TempBDT;
uint8_t usb_device_state;          // Device States: DETACHED, ATTACHED, ...
USB_DEVICE_STATUS usb_stat;        // Global USB flags (remote wakeup armed status, etc.)
uint8_t usb_active_cfg;            // Value of current configuration
uint8_t usb_alt_intf[MAX_NUM_INT]; // Array to keep track of the current alternate
                                   // setting for each interface ID

uint8_t USTATSave;
uint16_t Counter;
bool DeviceIsSoftDetached;

#define BDT_ADDR    0x20
#define USB_RAM_BUFF_ADDR   0x40    //EP0 OUT and IN buffers
#define USB_HID_BUFF_OUT_ADDR  @0xA0
#define USB_HID_BUFF_IN_ADDR   @0x120

#ifndef __XC8__
    #pragma udata USB_BDT = BDT_ADDR
#else
    #ifndef USB_RAM_BUFF_ADDR
        #define USB_RAM_BUFF_ADDR   (BDT_ADDR + 12 + (MAX_EP_NUMBER * 8))
    #endif
#endif

#define BDT_ADDR_TAG    @BDT_ADDR
#define BDT_ADDR_TAG_EP0O_EVEN   @BDT_ADDR
#define BDT_ADDR_TAG_EP0O_ODD    BDT_ADDR_TAG_EP0O_EVEN+4
#define BDT_ADDR_TAG_EP0I   BDT_ADDR_TAG+8
#define BDT_ADDR_TAG_EP1O   BDT_ADDR_TAG+12
#define BDT_ADDR_TAG_EP1I   BDT_ADDR_TAG+16
#define USB_EP0_BUFF_ADDR   @USB_RAM_BUFF_ADDR
#define USB_EP0_BUFF_ADDR2  USB_EP0_BUFF_ADDR+EP0_BUFF_SIZE
#define USB_CTRL_TRF_DATA_ADDR USB_EP0_BUFF_ADDR2+EP0_BUFF_SIZE
#ifndef USB_HID_BUFF_OUT_ADDR
#define USB_HID_BUFF_OUT_ADDR  USB_CTRL_TRF_DATA_ADDR+EP0_BUFF_SIZE
#define USB_HID_BUFF_IN_ADDR   USB_HID_BUFF_OUT_ADDR+HID_INT_OUT_EP_SIZE
#endif


#if(0 <= MAX_EP_NUMBER)
volatile BDT ep0BoEven BDT_ADDR_TAG_EP0O_EVEN;         //Endpoint #0 BD Out EVEN
volatile BDT ep0BoOdd BDT_ADDR_TAG_EP0O_ODD;          //Endpoint #0 BD Out ODD
volatile BDT ep0Bi BDT_ADDR_TAG_EP0I;         //Endpoint #0 BD In
#endif

#if(1 <= MAX_EP_NUMBER)
volatile BDT ep1Bo BDT_ADDR_TAG_EP1O;         //Endpoint #1 BD Out
volatile BDT ep1Bi BDT_ADDR_TAG_EP1I;         //Endpoint #1 BD In
#endif

#if(2 <= MAX_EP_NUMBER)
volatile BDT ep2Bo;         //Endpoint #2 BD Out
volatile BDT ep2Bi;         //Endpoint #2 BD In
#endif

#if(3 <= MAX_EP_NUMBER)
volatile BDT ep3Bo;         //Endpoint #3 BD Out
volatile BDT ep3Bi;         //Endpoint #3 BD In
#endif

#if(4 <= MAX_EP_NUMBER)
volatile BDT ep4Bo;         //Endpoint #4 BD Out
volatile BDT ep4Bi;         //Endpoint #4 BD In
#endif

#if(5 <= MAX_EP_NUMBER)
volatile far BDT ep5Bo;         //Endpoint #5 BD Out
volatile far BDT ep5Bi;         //Endpoint #5 BD In
#endif

#if(6 <= MAX_EP_NUMBER)
volatile far BDT ep6Bo;         //Endpoint #6 BD Out
volatile far BDT ep6Bi;         //Endpoint #6 BD In
#endif

#if(7 <= MAX_EP_NUMBER)
volatile far BDT ep7Bo;         //Endpoint #7 BD Out
volatile far BDT ep7Bi;         //Endpoint #7 BD In
#endif

#if(8 <= MAX_EP_NUMBER)
volatile far BDT ep8Bo;         //Endpoint #8 BD Out
volatile far BDT ep8Bi;         //Endpoint #8 BD In
#endif

#if(9 <= MAX_EP_NUMBER)
volatile far BDT ep9Bo;         //Endpoint #9 BD Out
volatile far BDT ep9Bi;         //Endpoint #9 BD In
#endif

#if(10 <= MAX_EP_NUMBER)
volatile far BDT ep10Bo;        //Endpoint #10 BD Out
volatile far BDT ep10Bi;        //Endpoint #10 BD In
#endif

#if(11 <= MAX_EP_NUMBER)
volatile far BDT ep11Bo;        //Endpoint #11 BD Out
volatile far BDT ep11Bi;        //Endpoint #11 BD In
#endif

#if(12 <= MAX_EP_NUMBER)
volatile far BDT ep12Bo;        //Endpoint #12 BD Out
volatile far BDT ep12Bi;        //Endpoint #12 BD In
#endif

#if(13 <= MAX_EP_NUMBER)
volatile far BDT ep13Bo;        //Endpoint #13 BD Out
volatile far BDT ep13Bi;        //Endpoint #13 BD In
#endif

#if(14 <= MAX_EP_NUMBER)
volatile far BDT ep14Bo;        //Endpoint #14 BD Out
volatile far BDT ep14Bi;        //Endpoint #14 BD In
#endif

#if(15 <= MAX_EP_NUMBER)
volatile far BDT ep15Bo;        //Endpoint #15 BD Out
volatile far BDT ep15Bi;        //Endpoint #15 BD In
#endif

volatile uint8_t EP0OutEvenBuf[EP0_BUFF_SIZE] USB_EP0_BUFF_ADDR;
volatile uint8_t EP0OutOddBuf[EP0_BUFF_SIZE] USB_EP0_BUFF_ADDR2;
volatile CTRL_TRF_DATA CtrlTrfData USB_CTRL_TRF_DATA_ADDR;


volatile unsigned char hid_report_out[HID_INT_OUT_EP_SIZE] USB_HID_BUFF_OUT_ADDR;
volatile unsigned char hid_report_in[HID_INT_IN_EP_SIZE] USB_HID_BUFF_IN_ADDR;


#ifndef __XC8__
#pragma udata
#endif



#if !defined(USE_USB_BUS_SENSE_IO)
    #define usb_bus_sense       1
#endif

#if !defined(USE_SELF_POWER_SENSE_IO)
    #define self_power          0
#endif



#ifndef __XC8__
#pragma code
#endif


void usb_device_init(void)
{
    if(UCONbits.USBEN == 1)
    {
        usb_disable_with_long_delay();
    }
    DeviceIsSoftDetached = FALSE;
    usb_check_bus_status();
}

void usb_soft_attach(void)
{
    if(DeviceIsSoftDetached == TRUE)
    {
        usb_disable_with_long_delay();
    }

    UCON = 0;
    UCFG = UCFG_VAL;
    UIE = 0;
    UCONbits.USBEN = 1;

    usb_protocol_reset_handler();
    usb_device_state = ATTACHED_STATE;
    DeviceIsSoftDetached = FALSE;
}

void usb_soft_detach(void)
{
    UCONbits.SUSPND = 0;    
    UCON = 0x00;
    usb_device_state = DETACHED_STATE;
    DeviceIsSoftDetached = TRUE;
}

void usb_check_bus_status(void)
{
    if(DeviceIsSoftDetached == TRUE)
    {
        return;
    }

    #define USB_BUS_ATTACHED    1
    #define USB_BUS_DETACHED    0

    #ifdef USE_USB_BUS_SENSE_IO
        if(usb_bus_sense == USB_BUS_ATTACHED) {
            if(UCONbits.USBEN == 0) {
                usb_soft_attach();
            }
        }
        else {
            if(UCONbits.USBEN == 1) {
                usb_soft_detach();
                DeviceIsSoftDetached = FALSE;
            }
        }
    #else
        if(UCONbits.USBEN == 0)
            usb_soft_attach();
    #endif
}

void usb_device_tasks(void)
{
	static volatile BDT* pBDTEntry;
	static uint8_t i;
	
	usb_check_bus_status();

	if(usb_device_state == DETACHED_STATE) {
		return;
	} 
	
	if(UIRbits.ACTVIF)    usb_wake_from_suspend();
	
	/*
	* Pointless to continue servicing if the device is in suspend mode.
	*/
	if(UCONbits.SUSPND == 1) {
		return;
	}
	if(UIRbits.URSTIF)    usb_protocol_reset_handler();
	
	if(UIRbits.IDLEIF) {
/* Rilassaru w.a.
		usb_suspend();
*/
	}
	
	if(usb_device_state < DEFAULT_STATE) return;
	
    for(bTRNIFCount = 0; bTRNIFCount < 4; bTRNIFCount++)
    {
        if(UIRbits.TRNIF)
        {
            USTATSave = USTAT;
            if((USTAT & 0x7C) == EP00_OUT)
            {
                if(USTATbits.PPBI == 0)
                {
                    pBDTEntry = &ep0BoEven;
                }
                else
                {
                    pBDTEntry = &ep0BoOdd;
                }

                UIRbits.TRNIF = 0;

                if(pBDTEntry->Stat.PID == SETUP_TOKEN)
                {
                    for(i = 0; i < sizeof(CTRL_TRF_SETUP); i++)
                    {
                        SetupPkt._byte[i] = *pBDTEntry->ADR++;
                    }

                    usb_ctrl_trf_setup_handler();
                }
                else
                {
                    usb_ctrl_trf_out_handler(USTATSave);
                }
            }
            else if(USTAT == EP00_IN)
            {
                UIRbits.TRNIF = 0;
                usb_ctrl_trf_in_handler();
            }
            else
            {
                UIRbits.TRNIF = 0;
            }
        }//if(UIRbits.TRNIF)
        else
        {
            break;
        }
    }

}

void usb_suspend(void)
{
    static unsigned char UIESave;

    UIESave = UIE;
    UIE = 0b00000100;
    UIRbits.IDLEIF = 0;
    UCONbits.SUSPND = 1;
    
    USBIF_FLAG = 0;
    USBIE_BIT = 1;

    usb_cb_suspend();

    USBIE_BIT = 0;
    UIE |= UIESave;
}

void usb_wake_from_suspend(void)
{
    usb_cb_wake_from_suspend();

    UCONbits.SUSPND = 0;
    UIEbits.ACTVIE = 0;
    while(UIRbits.ACTVIF){UIRbits.ACTVIF = 0;}

}

void usb_protocol_reset_handler(void)
{
    usb_device_state = DEFAULT_STATE;
    UEIE = 0;                       // Not using USB error interrupts (no special handling required anyway)
    UIR = 0;                        // Clears all USB interrupts
    UIE = 0b01111011;               // Enable all interrupts except ACTVIE
    UADDR = 0x00;                   // Reset to default address
    mDisableEP1to7();               // Reset all non-EP0 UEPn registers
    UEP0 = EP_CTRL|HSHK_EN;         // Init EP0 as a Ctrl EP, see usb_device.h
    UCONbits.PPBRST = 1;            // Reset ping pong buffer pointers
    while(UIRbits.TRNIF == 1)       // Flush any pending transactions
    {
        UIRbits.TRNIF = 0;
        ClrWdt();    //5 Tcy minimum (2 for call, 2 for return, 1 for clearing) to allow TRNIF to (potentially) reassert
    }
    UCONbits.PPBRST = 0;
    UCONbits.PKTDIS = 0;            // Make sure packet processing is enabled

    //Prepare EP0 OUT Even to receive the first SETUP packet
    TempBDT.Stat._byte = _DAT0|_BSTALL;
    load_bdt_and_set_uown(EP0_OUT_EVEN_BDT_INDEX);    //Configures address/size fields and sets UOWN
    EP0OutOddNeedsArmingNext = TRUE;
    usb_stat._byte = 0x00;          // Clear USB flags (like remote wakeup armed status)
    usb_active_cfg = 0;             // Clear active configuration
    usb_cb_init_ep(0);                 // Call application callback function to give it notification
                                    // it is getting un-configured (ex: equiv of set configuration to 0).
}

void usb_ctrl_trf_setup_handler(void)
{
    ep0Bi.Stat._byte = _UCPU;           
    short_pkt_status = SHORT_PKT_NOT_SENT;

    //Make sure none of the EP0 OUT endpoints are still armed (one could still
    //be UOWN == 1, if EP0 OUT was previously double armed).
    //Clear UOWN on all EP0 OUT BDTs until we are done parsing/processing this SETUP.
    if(ep0BoEven.Stat.UOWN == 1)
    {
        ep0BoEven.Stat._byte = _UCPU;
        EP0OutOddNeedsArmingNext = FALSE;
    }
    if(ep0BoOdd.Stat.UOWN == 1)
    {
        ep0BoOdd.Stat._byte = _UCPU;
        EP0OutOddNeedsArmingNext = TRUE;
    }
    ctrl_trf_state = WAIT_SETUP;
    ctrl_trf_session_owner = MUID_NULL;
    wCount.Val = 0;
    UCONbits.PKTDIS = 0;

    usb_check_std_request();
    usb_cb_check_other_req();

    usb_ctrl_ep_service_complete();

}

#if defined (ENABLE_CONTROL_TRANSFERS_WITH_OUT_DATA_STAGE)
void usb_ctrl_trf_out_handler(uint8_t USTATValue)
{
    uint8_t bytes_received;

    if(ctrl_trf_state == CTRL_TRF_RX)
    {
        if(USTATValue & 0x02)
        {
            bytes_received = ep0BoOdd.Cnt;
            pSrc.bRam = ep0BoOdd.ADR;
        }
        else
        {
            bytes_received = ep0BoEven.Cnt;
            pSrc.bRam = ep0BoEven.ADR;
        }
        
        wCount.Val = wCount.Val + bytes_received;

        while(bytes_received)
        {
            *pDst.bRam++ = *pSrc.bRam++;
            bytes_received--;
        }

        if(wCount.Val < SetupPkt.wLength)
        {
            if(EP0OutOddNeedsArmingNext == TRUE)
            {
                if(ep0BoEven.Stat.DTS == 1)
                {
                    TempBDT.Stat._byte = _DAT0 | _DTSEN;
                }
                else
                {
                    TempBDT.Stat._byte = _DAT1 | _DTSEN;
                }
                load_bdt_and_set_uown(EP0_OUT_ODD_BDT_INDEX);
                EP0OutOddNeedsArmingNext = FALSE;
            }
            else
            {
                if(ep0BoOdd.Stat.DTS == 1)
                {
                    TempBDT.Stat._byte = _DAT0 | _DTSEN;
                }
                else
                {
                    TempBDT.Stat._byte = _DAT1 | _DTSEN;
                }
                load_bdt_and_set_uown(EP0_OUT_EVEN_BDT_INDEX);
                EP0OutOddNeedsArmingNext = TRUE;
            }
        }
        else
        {
            TempBDT.Stat._byte = _BSTALL;
            if(EP0OutOddNeedsArmingNext == TRUE)
            {
                load_bdt_and_set_uown(EP0_OUT_ODD_BDT_INDEX);
                EP0OutOddNeedsArmingNext = FALSE;
            }
            else
            {
                load_bdt_and_set_uown(EP0_OUT_EVEN_BDT_INDEX);
                EP0OutOddNeedsArmingNext = TRUE;
            }

            ep0Bi.Cnt = 0;
            ep0Bi.Stat._byte = _DAT1|_DTSEN;
            ep0Bi.Stat._byte |= _USIE;


            USBCBControlTransferOutDataReady();
        }
    }
    else
    {
    }
}
#endif


void usb_ctrl_trf_in_handler(void)
{
    if(usb_device_state == ADR_PENDING_STATE)
    {
        UADDR = SetupPkt.bDevADR;
        if(UADDR > 0)
            usb_device_state = ADDRESS_STATE;
        else
            usb_device_state = DEFAULT_STATE;
    }

    if(ctrl_trf_state == CTRL_TRF_TX)
    {
        usb_ctrl_trf_tx_service();

        if(short_pkt_status == SHORT_PKT_SENT)
        {
            ep0Bi.Stat._byte = _BSTALL;
            ep0Bi.Stat._byte |= _USIE;
        }
        else
        {
            if(ep0Bi.Stat.DTS == 0)
                ep0Bi.Stat._byte = _DAT1|_DTSEN;
            else
                ep0Bi.Stat._byte = _DAT0|_DTSEN;

            ep0Bi.Stat._byte |= _USIE;
        }
    }
    else
    {
    }

}

void usb_ctrl_trf_tx_service(void)
{
    static uint8_t bytes_to_send;

    bytes_to_send = EP0_BUFF_SIZE;
    if(wCount.Val < EP0_BUFF_SIZE)
    {
        bytes_to_send = wCount.Val;
        if(short_pkt_status == SHORT_PKT_NOT_SENT)
        {
            short_pkt_status = SHORT_PKT_PENDING;
        }
        else if(short_pkt_status == SHORT_PKT_PENDING)
        {
            short_pkt_status = SHORT_PKT_SENT;
        }
    }

    ep0Bi.Cnt = bytes_to_send;
    wCount.Val -= bytes_to_send;

    pDst.bRam = (uint8_t*)&CtrlTrfData;
    if(usb_stat.ctrl_trf_mem == _ROM)
    {
        while(bytes_to_send)
        {
            *pDst.bRam = *pSrc.bRom;
            pDst.bRam++;
            pSrc.bRom++;
            bytes_to_send--;
        }
    }
    else
    {
        while(bytes_to_send)
        {
            *pDst.bRam = *pSrc.bRam;
            pDst.bRam++;
            pSrc.bRam++;
            bytes_to_send--;
        }
    }

}

void usb_ctrl_ep_service_complete(void)
{
    if(ctrl_trf_session_owner == MUID_NULL)
    {
        ep0Bi.Stat._byte = _BSTALL;
        ep0Bi.Stat._byte |= _USIE;
        TempBDT.Stat._byte = _BSTALL;
        if(EP0OutOddNeedsArmingNext == TRUE)
        {
            load_bdt_and_set_uown(EP0_OUT_ODD_BDT_INDEX);
            EP0OutOddNeedsArmingNext = FALSE;
        }
        else
        {
            load_bdt_and_set_uown(EP0_OUT_EVEN_BDT_INDEX);
            EP0OutOddNeedsArmingNext = TRUE;
        }
    }
    else
    {
        if(SetupPkt.DataDir == DEV_TO_HOST)
        {
            ctrl_trf_state = CTRL_TRF_TX;

            if(SetupPkt.wLength < wCount.Val)
                wCount.Val = SetupPkt.wLength;

            usb_ctrl_trf_tx_service();

            TempBDT.Stat._byte = _DAT1 | _DTSEN;  //DTS = 1 for the status stage, DTS ignored/irrelevant for SETUP packets
            load_bdt_and_set_uown(EP0_OUT_ODD_BDT_INDEX);
            load_bdt_and_set_uown(EP0_OUT_EVEN_BDT_INDEX);

            ep0Bi.ADR = (uint8_t*)&CtrlTrfData;
            ep0Bi.Stat._byte = _DAT1|_DTSEN;
            ep0Bi.Stat._byte |= _USIE;
        }//if(SetupPkt.DataDir == DEV_TO_HOST)
        else    //else we must be (SetupPkt.DataDir == HOST_TO_DEV)
        {
            /*
             * Control Write (with data stage):
             * <SETUP[0]><OUT[1]><OUT[0]>...<IN[1]> | <SETUP[0]>
             *
             * Certain host to device requests may not have any data stage, such
             * as the "set address" request:
             * <SETUP[0]> <IN[1]> | <SETUP[0]>
             */

            //Keep track of what we are doing, accross multiple USB packets.
            ctrl_trf_state = CTRL_TRF_RX;

            //Prepare OUT EP to receive either the first DATA1 OUT data packet in the host
            //to device control transfer, or the SETUP packet (if no data stage).
            //We only arm one of the EP0 OUT buffers for this.
            TempBDT.Stat._byte = _BSTALL;   //Assume initially we will get a SETUP
            //Check the length of the transfer, if is not 0, then the next packet will be a normal OUT instead
            if(SetupPkt.wLength == 0)
            {
                TempBDT.Stat._byte = _DAT1|_DTSEN;    //Prepare for normal OUT packet instead
            }
            //Check which EP0 out needs arming, and arm it.
            if(EP0OutOddNeedsArmingNext == TRUE)
            {
                load_bdt_and_set_uown(EP0_OUT_ODD_BDT_INDEX);
                EP0OutOddNeedsArmingNext = FALSE;
            }
            else
            {
                load_bdt_and_set_uown(EP0_OUT_EVEN_BDT_INDEX);
                EP0OutOddNeedsArmingNext = TRUE;
            }

            if(SetupPkt.wLength == 0)
            {
                //Arm the status stage 0-byte IN packet
                ep0Bi.Cnt = 0;
                ep0Bi.Stat._byte = _DAT1|_DTSEN;
                ep0Bi.Stat._byte |= _USIE;
            }
        }
    }
}


void usb_check_std_request(void)
{
    if(SetupPkt.RequestType != STANDARD) return;

    switch(SetupPkt.bRequest)
    {
        case SET_ADR:
            ctrl_trf_session_owner = MUID_USB9;
            usb_device_state = ADR_PENDING_STATE;       // Update state only
            break;
        case GET_DSC:
            usb_std_get_dsc_handler();
            break;
        case SET_CFG:
            usb_std_set_cfg_handler();
            break;
        case GET_CFG:
            ctrl_trf_session_owner = MUID_USB9;
            pSrc.bRam = (uint8_t*)&usb_active_cfg;         // Set Source
            usb_stat.ctrl_trf_mem = _RAM;               // Set memory type
            //LSB(wCount) = 1;                            // Set data count
            wCount.v[0] = 1;
            break;
        case GET_STATUS:
            usb_std_get_status_handler();
            break;
        case CLR_FEATURE:
        case SET_FEATURE:
            usb_std_feature_req_handler();
            break;
        case GET_INTF:
            ctrl_trf_session_owner = MUID_USB9;
            pSrc.bRam = (uint8_t*)&usb_alt_intf+SetupPkt.bIntfID;  // Set source
            usb_stat.ctrl_trf_mem = _RAM;               // Set memory type
            wCount.v[0] = 1;                            // Set data count
            break;
        case SET_INTF:
            ctrl_trf_session_owner = MUID_USB9;
            usb_alt_intf[SetupPkt.bIntfID] = SetupPkt.bAltID;
            break;
        case SET_DSC:
        case SYNCH_FRAME:
        default:
            break;
    }//end switch

}


void usb_std_get_dsc_handler(void)
{
    if(SetupPkt.bmRequestType == 0x80)
    {
        switch(SetupPkt.bDscType)
        {
            case DSC_DEV:
                ctrl_trf_session_owner = MUID_USB9;
                pSrc.bRom = (ROM uint8_t*)&device_dsc;
                wCount.v[0] = sizeof(device_dsc);          // Set data count
                break;
            case DSC_CFG:
                if(SetupPkt.bDscIndex < USB_MAX_NUM_CONFIG_DSC)
                {
                    ctrl_trf_session_owner = MUID_USB9;
                    pSrc.bRom = (ROM BYTE*)&CFG01;
                    wCount.Val = sizeof(CFG01);              // Set data count
                }
                break;
            case DSC_STR:
                ctrl_trf_session_owner = MUID_USB9;
                pSrc.bRom = *(USB_SD_Ptr+SetupPkt.bDscIndex);
                wCount.Val = *pSrc.bRom;
                break;
        }
        usb_stat.ctrl_trf_mem = _ROM;
    }
}
void usb_std_set_cfg_handler(void)
{
    static unsigned char i;
    
    ctrl_trf_session_owner = MUID_USB9;
    mDisableEP1to7();                          // See usb_device.h
    for(i = 0; i < MAX_NUM_INT; i++)
    {
        usb_alt_intf[i] = 0;
    }

    usb_active_cfg = SetupPkt.bCfgValue;

    usb_cb_init_ep(usb_active_cfg);
    
    if(SetupPkt.bCfgValue == 0)
    {
        usb_device_state = ADDRESS_STATE;
    }
    else
    {
        usb_device_state = CONFIGURED_STATE;
    }//end if(SetupPkt.bcfgValue == 0)
}

void usb_std_get_status_handler(void)
{
    CtrlTrfData._byte0 = 0;                         // Initialize content
    CtrlTrfData._byte1 = 0;

    switch(SetupPkt.Recipient)
    {
        case RCPT_DEV:
            ctrl_trf_session_owner = MUID_USB9;
            /*
             * _byte0: bit0: Self-Powered Status [0] Bus-Powered [1] Self-Powered
             *         bit1: RemoteWakeup        [0] Disabled    [1] Enabled
             */

            if(self_power == 1)                     // self_power defined in HardwareProfile.h
                CtrlTrfData._byte0 |= 0b00000001;   // Set bit0
            if(usb_stat.RemoteWakeup == 1)          // usb_stat defined in usbmmap.c
                CtrlTrfData._byte0|=0b00000010;     // Set bit1
            break;
        case RCPT_INTF:
            ctrl_trf_session_owner = MUID_USB9;     // No data to update
            break;
        case RCPT_EP:
            ctrl_trf_session_owner = MUID_USB9;
            /*
             * _byte0: bit0: Halt Status [0] Not Halted [1] Halted
             */
            pDst.bRam = (uint8_t*)&ep0BoEven+(SetupPkt.EPNum*8)+(SetupPkt.EPDir*4)+4;   //+4 is to skip past the EP0 OUT ODD BDT entry
            if(*pDst.bRam & _BSTALL)    // Use _BSTALL as a bit mask
                CtrlTrfData._byte0=0x01;// Set bit0
            break;
    }//end switch

    if(ctrl_trf_session_owner == MUID_USB9)
    {
        pSrc.bRam = (uint8_t*)&CtrlTrfData;            // Set Source
        usb_stat.ctrl_trf_mem = _RAM;               // Set memory type
        wCount.v[0] = 2;                            // Set data count
    }//end if(...)
}


void usb_std_feature_req_handler(void)
{
    if((SetupPkt.bFeature == DEVICE_REMOTE_WAKEUP)&&(SetupPkt.Recipient == RCPT_DEV))
    {
        ctrl_trf_session_owner = MUID_USB9;
        if(SetupPkt.bRequest == SET_FEATURE)
            usb_stat.RemoteWakeup = 1;
        else
            usb_stat.RemoteWakeup = 0;
    }

    if((SetupPkt.bFeature == ENDPOINT_HALT)&&(SetupPkt.Recipient == RCPT_EP)&&(SetupPkt.EPNum != 0))
    {
        ctrl_trf_session_owner = MUID_USB9;
        pDst.bRam = (uint8_t*)&ep0BoEven+(SetupPkt.EPNum*8)+(SetupPkt.EPDir*4)+4;

        if(SetupPkt.bRequest == SET_FEATURE)
        {
            *pDst.bRam = _BSTALL;
            *pDst.bRam |= _USIE;
        }
        else
        {
            if(SetupPkt.EPDir == 1) // IN
                *pDst.bRam = _UCPU|_DAT1;
            else
            {
                *pDst.bRam = _DAT0|_DTSEN;
                *pDst.bRam |= _USIE;
            }
        }//end if
    }//end if
}

void load_bdt_and_set_uown(uint8_t BDTIndexToLoad)
{
    static volatile BDT* pBDTEntry;

    TempBDT.Cnt = EP0_BUFF_SIZE;
    TempBDT.ADR = (uint8_t*)&EP0OutOddBuf[0];
    if(BDTIndexToLoad == EP0_OUT_EVEN_BDT_INDEX)
    {
        TempBDT.ADR = (uint8_t*)&EP0OutEvenBuf[0];
        pBDTEntry = (volatile BDT*)BDT_ADDR;
    }
    else
    {
        pBDTEntry = (volatile BDT*)(BDT_ADDR + 4);
    }

    *pBDTEntry = TempBDT;

    pBDTEntry->Stat.UOWN = 1;
}

void usb_disable_with_long_delay(void)
{
    UCONbits.SUSPND = 0;    //Make sure not in suspend mode
    UCON = 0x00;            //Disable USB module
    delay_routine(0xFFFF);   //Wait long time for host to recognize detach event
    usb_device_state = DETACHED_STATE;
}

void delay_routine(unsigned int DelayAmount)
{
    while(DelayAmount)
    {
        ClrWdt();
        DelayAmount--;
    }
}
