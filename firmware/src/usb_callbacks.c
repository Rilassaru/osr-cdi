/* ----------------------------------------------------------------------------
  Open Source Replica CDI 'OSR-CDI' system for YAMAHA 2T motorcycle
  ----------------------------------------------------------------------------
Copyright(c) 2013 - 2024, Rilassaru(http://rilassaru.blog.jp/)
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
#include "userinterface.h"

void usb_cb_wake_from_suspend(void) {
/* This code for CDI thererore do not use USB suspend.
    delay_routine(0x300); //Device will switch clocks (if using two-speed startup) while executing this delay function
-- end of rilassaru -- */
}

void usb_cb_suspend(void) {
/* Rilassaru: CDI does not use USB suspend.

    Sleep();
    while ((USBIF_FLAG == 0))
    {
        Sleep(); //Entry into sleep clears WDT count, much like executing ClrWdt() instruction
    }
-- end of rilassaru -- */
}

void usb_cb_init_ep(uint8_t ConfigurationIndex) {
    if (ConfigurationIndex == 1) //This application only implements one configuration, with index == 1.
    {
        hid_init_ep();
        user_init();
    }
}

void usb_cb_check_other_req(void) {
    usb_check_hid_request();
}
