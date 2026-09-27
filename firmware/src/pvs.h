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
#ifndef PVS__H
#define	PVS__H

/** -----------------------------------------------------------------
 * Type definition
------------------------------------------------------------------ */
typedef enum{
	MTD_OPEN,
	MTD_CLOSE,
	MTD_STOP,
	MTD_BRAKE
}MOTOR_STATE;

// YPVS state
typedef enum{
	PVS_ACTIVE = 0,    // Engine started
	PVS_INIT_OPEN1,    // Open at first
	PVS_INIT_CLOSE1,   // Close after open
	PVS_INIT_OPEN2,    // Reopen after close
	PVS_INIT_CLOSE2    // Rclose after open (optional)
}YPVS_STATE;

/** -----------------------------------------------------------------
 * YPVS definition
------------------------------------------------------------------ */
#define PV_LEVEL_BUFFER_NUM (5) // Data size of NoiseReduction
#define NR_TRIGGER (8)          // Range to determine the electrical noise 
                                //(Decide for actual measurement) 

#define PVS_POS_MAX  (160)
#define PVS_POS_MIN (5)

#define MT_BRAKE()	{gpioPVSA=1;gpioPVSB=1;}
#define MT_OPEN()	{gpioPVSA=1;gpioPVSB=0;}
#define MT_CLOSE()	{gpioPVSA=0;gpioPVSB=1;}
#define MT_STOP()	{gpioPVSA=0;gpioPVSB=0;}

#define ENGINE_STOP_SC  (8000)  // Count to determine engine stop
#define ENGINE_START_SC (4000)  // Count to determine engine start

/** -----------------------------------------------------------------
 * global variable
------------------------------------------------------------------ */
//extern usbPacket g_status;

/** -----------------------------------------------------------------
 * Funciton proto type
------------------------------------------------------------------ */
void initialize_pvs_min_max(void);
uint8_t pvs_leveled_analog_data(uint8_t pot);
void pvs_motor_head(uint16_t rpm, uint8_t pot);

#endif	/* PVS__H */