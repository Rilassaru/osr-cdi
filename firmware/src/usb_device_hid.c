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

#ifndef __XC8__
#pragma udata
#endif
uint8_t idle_rate;
uint8_t active_protocol;               // [0] Boot Protocol [1] Report Protocol
uint8_t hid_rpt_rx_len;

void hid_get_report_handler(void);
void hid_set_report_handler(void);

#ifndef __XC8__
#pragma code
#endif

void usb_check_hid_request(void)
{
    if(SetupPkt.Recipient != RCPT_INTF) return;
    if(SetupPkt.bIntfID != HID_INTF_ID) return;
    
    if(SetupPkt.bRequest == GET_DSC)
    {
        switch(SetupPkt.bDscType)
        {
            case DSC_HID:
                ctrl_trf_session_owner = MUID_HID;
                pSrc.bRom = &CFG01[18];             //18 is a magic number (offset from start of configuration descriptor, to the start of the HID descriptor)
                wCount.Val = sizeof(USB_HID_DSC);
                break;
            case DSC_RPT:
                ctrl_trf_session_owner = MUID_HID;
                mUSBGetHIDRptDscAdr(pSrc.bRom);     // See usb_config.h
                mUSBGetHIDRptDscSize(wCount.Val); // See usb_config.h
                break;
            case DSC_PHY:
                // ctrl_trf_session_owner = MUID_HID;
                break;
        }//end switch(SetupPkt.bDscType)
        usb_stat.ctrl_trf_mem = _ROM;
    }//end if(SetupPkt.bRequest == GET_DSC)
    
    if(SetupPkt.RequestType != CLASS) return;
    switch(SetupPkt.bRequest)
    {
        case GET_REPORT:
            hid_get_report_handler();
            break;
        case SET_REPORT:
            hid_set_report_handler();            
            break;
        case GET_IDLE:
            ctrl_trf_session_owner = MUID_HID;
            pSrc.bRam = (uint8_t*)&idle_rate;      // Set source
            usb_stat.ctrl_trf_mem = _RAM;       // Set memory type
            wCount.v[0] = 1;                    // Set data count
            break;
        case SET_IDLE:
            ctrl_trf_session_owner = MUID_HID;
            //idle_rate = MSB(SetupPkt.W_Value);
            idle_rate = SetupPkt.W_Value.v[1];
            break;
        case GET_PROTOCOL:
            ctrl_trf_session_owner = MUID_HID;
            pSrc.bRam = (uint8_t*)&active_protocol;// Set source
            usb_stat.ctrl_trf_mem = _RAM;       // Set memory type
            wCount.v[0] = 1;                    // Set data count
            break;
        case SET_PROTOCOL:
            ctrl_trf_session_owner = MUID_HID;
            //active_protocol = LSB(SetupPkt.W_Value);
            active_protocol = SetupPkt.W_Value.v[0];
            break;
    }//end switch(SetupPkt.bRequest)

}

void hid_get_report_handler(void)
{
    // ctrl_trf_session_owner = MUID_HID;
}

void hid_set_report_handler(void)
{
    // ctrl_trf_session_owner = MUID_HID;
    // pDst.bRam = (byte*)&hid_report_out;
}


void hid_init_ep(void)
{   
    hid_rpt_rx_len =0;
    
    HID_UEP = EP_OUT_IN|HSHK_EN;                // Enable 2 data pipes

    //Arm the OUT interrupt endpoint so the host can send the first packet of data.
    HID_BD_OUT.Cnt = sizeof(hid_report_out);    // Set buffer size
    HID_BD_OUT.ADR = (uint8_t*)&hid_report_out; // Set buffer address
    HID_BD_OUT.Stat._byte = _DAT0|_DTSEN;       // Set status
    HID_BD_OUT.Stat._byte |= _USIE;

    HID_BD_IN.ADR = (uint8_t*)&hid_report_in;      // Set buffer address
    HID_BD_IN.Stat._byte = _UCPU|_DAT1;         // Set status

}


void hid_tx_report(char *buffer, uint8_t len)
{
    uint8_t i;
    
    /*
     * Value of len should be equal to or smaller than HID_INT_IN_EP_SIZE.
     * This check forces the value of len to meet the precondition.
     */
    if(len > HID_INT_IN_EP_SIZE)
        len = HID_INT_IN_EP_SIZE;

   /*
    * Copy data from user's buffer to a USB module accessible RAM packet buffer
    */
    for (i = 0; i < len; i++)
        hid_report_in[i] = buffer[i];

    HID_BD_IN.Cnt = len;
    mUSBBufferReady(HID_BD_IN);

}

uint8_t HIDRxReport(char *buffer, uint8_t len)
{
    hid_rpt_rx_len = 0;
    
    if(!mHIDRxIsBusy())
    {
        /*
         * Adjust the expected number of bytes to equal
         * the actual number of bytes received.
         */
        if(len > HID_BD_OUT.Cnt)
            len = HID_BD_OUT.Cnt;
        
        /*
         * Copy data from dual-ram buffer to user's buffer
         */
        for(hid_rpt_rx_len = 0; hid_rpt_rx_len < len; hid_rpt_rx_len++)
            buffer[hid_rpt_rx_len] = hid_report_out[hid_rpt_rx_len];

        /*
         * Prepare dual-ram buffer for next OUT transaction
         */
        HID_BD_OUT.Cnt = sizeof(hid_report_out);
        mUSBBufferReady(HID_BD_OUT);
    }//end if
    
    return hid_rpt_rx_len;
    
}
