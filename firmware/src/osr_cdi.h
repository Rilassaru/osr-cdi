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
#ifndef OSR_CDI__H
#define	OSR_CDI__H

#include "typedefs.h"
#include "constant.h"
#include "pvs.h"
#include "qshifter.h"

/** -----------------------------------------------------------------
 * Type definition
------------------------------------------------------------------ */
#define NUMBER_OF_MAP_CH            (4)     // number of maps.

#define MAP_MIN_RPM                 (1)		// map able min.
#define MAP_MAX_RPM                 (159)	// map table max.

#define SCR_GATE_ON_TIME_SHORT      (20)    // Thyristor open time(us)
#define SCR_GATE_ON_TIME_LONG       (200)   // Thyristor open time(us)
#define SCR_GATE_ON_HI_AREA         (60)    // Thyristor gate time switching threshold(rpm))

#define SCR_GATE_ON                 (1)     // Thyristor gate port on.
#define SCR_GATE_OFF                (0)     // Thyristor gate port off.

#define ENGINE_STATE_STARTED        (0)     // status of engine started.
#define ENGINE_STATE_STOPPED        (1)     // status of engine stopped.

#define ENABLE_2ND_WAVE_IGNITION    (1)     // enable 2nd wave ignition
#define DISABLE_2ND_WAVE_IGNITION   (0)     // enable 2nd wave ignition

#define REV_OVER_COUNT_MAX          (3)     // rev limitter, Over (n) times then cut.

// gCfg.sys_opt_port
enum{
    OPT_PORT_PULSE = 0,
    OPT_PORT_MAPSW
};

// gCfg.tp_type
enum{
    TP_TYPE_1CH = 0,
    TP_TYPE_2CH,
    TP_TYPE_4CH
};

/** -----------------------------------------------------------------
 * global variable
------------------------------------------------------------------ */
extern volatile uint8_t	gRPM;           // Rotaion per minutes	
extern volatile uint8_t	gEngineState;   // Stop or Started
extern volatile uint8_t gMapSelect;     // State of map sw device. 
extern volatile uint8_t	gQShifterState; // State of Quick shifter function.
extern volatile uint8_t	gQShifterCount; // Work counter for Q.S.

#endif	/* OSR_CDI__H */

