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
#ifndef USBCFG_H
#define USBCFG_H

#define MAX_EP_NUMBER           1   // EP0 and EP1 are the only EPs used in this application
#define MAX_NUM_INT             1   // For tracking Alternate Setting - make sure this matches the number of interfaces implemented in the device
#define EP0_BUFF_SIZE           8   // Valid Options: 8, 16, 32, or 64 bytes.
                                    // There is little advantage in using
                                    // more than 8 bytes on EP0 IN/OUT in most cases.
#define USB_MAX_NUM_CONFIG_DSC  1   // Number of configurations that this firmware implements
//#define ENABLE_CONTROL_TRANSFERS_WITH_OUT_DATA_STAGE    //Commented out to save code size, since this bootloader firmware doesn't use OUT control transfers with data stage


#define CONFIG_DESC_TOTAL_LEN    41     //Make sure this matches the size of your configuration descriptor + all subordinate
                                        //descriptors returned by the get descriptor(configuration) request

#define MODE_PP                 _PPBM1  //This code is only written to support _PPBM1 mode only (ping pong on EP0 OUT buffer only).  Do not change.
#define UCFG_VAL                _PUEN|_TRINT|_FS|MODE_PP

// HID
#define HID_INTF_ID             0x00
#define HID_UEP                 UEP1
#define HID_BD_OUT              ep1Bo
#define HID_INT_OUT_EP_SIZE     64
#define HID_BD_IN               ep1Bi
#define HID_INT_IN_EP_SIZE      64
#define HID_NUM_OF_DSC          1       //Just the Report descriptor (no physical descriptor present)
#define HID_RPT01_SIZE          29      //Make sure this matches the size of your HID report descriptor




#endif //USBCFG_H
