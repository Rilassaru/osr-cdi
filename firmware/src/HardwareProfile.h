/* ----------------------------------------------------------------------------
  Open Source Replica CDI 'OSR-CDI' system for YAMAHA 2T motorcycle
  ----------------------------------------------------------------------------
Copyright(c) 2013-, Rilassaru(http://rilassaru.blog.jp/)
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
#ifndef __HARDWARE_PROFILE_H_
#define __HARDWARE_PROFILE_H_

#define _XTAL_FREQ 16000000

//------------------------------------------------------------------
// PIN layout
//------------------------------------------------------------------
/*                   PIC16F1455
                     |VDD  VSS|
2nd pulse enabler << |RA5   D+| <> USB+(RA0)
SW1               >> |RA4   D-| <> USB-(RA1)
Q-SHIFTER         >> |RA3 Vusb| <> Vusb(RA2)
YPVS+             << |RC5  RC0| >> Thyristor gate
YPVS-             << |RC4  RC1| << INT PULSE IN
YPVS POT          >> |RC3  RC2| << utility port
*/
#define gpioSCRGATE             LATC0	// Digital in
#define gpioPICKUP              RC1		// Digital in
#define gpioOPTPORT             RC2		// Analog in/Degital out
//      YPVS pos                RC3 	// Analog in
#define gpioPVSA               LATC4	// Digital out
#define gpioPVSB                LATC5	// Digital out
//      USB+                    A0
//      USB-                    A1
//      USB Vbus                A2
#define gpioSHIFTER             RA3		// Digital in
#define gpioSW1                 RA4		// Digital in
#define gpioANALOG_PU_ENABLER   LATA5	// Digital out

#define CHS_THP (0b00011)       // Pin RA4/AN3
#define CHS_PVS (0b00111)       // Pin RC3/AN7

#endif //__HARDWARE_PROFILE_H_
