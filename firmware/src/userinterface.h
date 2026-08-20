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
#ifndef USERINTERFACE_H
#define USERINTERFACE_H

void user_init(void);
void hid_user_interface(void);

#define IDLE				0x00
#define NOT_IDLE			0x01

#define USB_PACKET_SIZE		0x40    // 64dec

// ------------------------------------------------------------------------------------------
// Packet types
// ------------------------------------------------------------------------------------------
typedef struct _global
{
	uint16_t    rpm;        // 01-02
	uint8_t     current_map;// 03
	uint8_t     e_stop;     // 04
	uint8_t     pv_pot;     // 05
	uint8_t     pv_target;  // 06
	uint8_t     pv_mode;    // 07
	uint8_t     pv_mt_state;// 08
	uint16_t	timer1;     // 09-10
	uint8_t     qs_signal;  // 11
	uint8_t     qs_state;   // 12
	uint8_t     tp_pot;     // 13
    uint8_t     reserve14;  // 14
	uint16_t    cpu_temp;   // 15-16
} global;

typedef union
{
	uint8_t	Command;
	uint8_t	Contents[USB_PACKET_SIZE];
	global	g;
} usbPacket;

usbPacket status;

#endif //USERINTERFACE_H
