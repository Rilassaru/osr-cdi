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
#ifndef CONSTANT_H
#define CONSTANT_H

#define ADDR_IG_TABLE		0x1840
#define ADDR_PV_TABLE		0x1D40
#define ADDR_CONFIG         0x1FC0
#define ADDR_PAGE_MAX		61

typedef struct _CONFIG{
	uint8_t major_version;              // 1
	uint8_t minor_version;              // 2
	uint8_t qs_enable_rpm;          	// 3
	uint8_t qs_sw_on_count;             // 4
	uint8_t qs_cut_count;               // 5
	uint8_t qs_disable_count;           // 6
	uint8_t qs_use_as_stopsw;       	// 7
	uint8_t qs_reverse_onoff_state;     // 8
	uint8_t rev_limit;                  // 9
	uint8_t rev_reserved01;             // 10
	uint16_t sys_numerator_for_rpm;     // 11-12
	uint8_t sys_pulse_per_rotation;     // 13
	uint8_t sys_pickup_degree;          // 14
	uint8_t count_of_an_ignition;       // 15
	uint8_t sys_opt_port;				// 16

	uint8_t tp_type;					// 17
	uint8_t tp_threshold01;				// 18
	uint8_t tp_threshold02;				// 19
	uint8_t tp_threshold03;				// 20

	uint8_t sys_rpm_for_an_ignition;    // 21
	uint8_t sys_pvs_init_pattern;       // 22
	uint8_t sys_reserved023;            // 23
	uint8_t sys_reserved024;            // 24
	uint8_t sys_reserved025;            // 25
	uint8_t sys_reserved026;            // 26
	uint8_t sys_reserved027;            // 27
	uint8_t sys_reserved028;            // 28
	uint8_t sys_reserved029;            // 29
	uint8_t sys_reserved030;            // 30
	uint8_t sys_reserved031;            // 31
	uint8_t sys_reserved032;            // 32

	uint8_t dummy[32];                  // 33-64
} CONFIG;

extern const uint16_t   gIGtbl[4][160];
extern const uint8_t    gPVtbl[4][160];
extern const uint8_t   gIgnoreNoiseTimer[160];
extern const CONFIG     gCfg;

/* -----------------------------------------------------------------
 * NOTE:
 * To calculate the number of revolutions, choose either.
 * One signal is generated per one clank rotation.
 * DT200/SDR etc.. 37,500
 * Two signal is generated per one clank rotation.
 * RZ250/TZR250/R1-Z etc.. 18,750
// ---------------------------------------------------------------*/

#endif