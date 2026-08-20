/* ----------------------------------------------------------------------------
Open Source Racing CDI 'OSR-CDI' system for YAMAHA 2T motorcycle
----------------------------------------------------------------------------
Copyright(c) 2013 - 2026, Rilassaru(http://rilassaru.blog.jp/)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met :

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and / or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
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

#pragma once
#include "hidapi.h"
#include <stdint.h>

#define HID_MAX_TXBUF_SIZE	65
#define HID_MAX_RXBUF_SIZE	64
#define MAX_STR				255
#define MAX_DATA_NUM		160

#define DEVICE_PID	0x003C
#define DEVICE_VID	0x04d8

// Specific setting value
#define COUNT_OF_TUNE_1	610
#define COUNT_OF_TUNE_2	570
#define PICKUP_DEG_1	68
#define PICKUP_DEG_2	36
#define SYS_NUMERATOR_FOR_RPM_1	37500
#define SYS_NUMERATOR_FOR_RPM_2	18750

// CDI command
#define CMD_GET_DATA		0x90
#define CMD_SET_DATA		0x92
#define CMD_GET_STATUS		0x93

//#define CMD_ERASE_DATA		0x91
//#define CMD_GET_DEVIDS		0x94
/* Version1.4.x command
// CDI command
#define CMD_GET_DATA		0x80
#define CMD_ERASE_DATA		0x81
#define CMD_SET_DATA		0x82
#define CMD_GET_STATUS		0x83
#define CMD_GET_DEVIDS		0x84
*/
#define RET_HID_CMD_SUCCESS 0x00
#define RET_HID_CMD_FAIL    0xFF

typedef void (MsgCallback)(const char*);

#pragma pack(push, 1)

typedef struct _status
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
	uint8_t		reserve14;	// 14
	uint16_t    cpu_temp;   // 15-16
} status;

typedef struct _devids
{
	uint16_t userid0;
	uint16_t userid1;
	uint16_t userid2;
	uint16_t userid3;
	uint16_t reserved1;
	uint16_t revid;
	uint16_t devid;
} devids;


typedef struct _IG_DATA
{
	uint8_t		bin[4];
	uint16_t	data[16];
} IG_DATA;

typedef struct _PV_DATA
{
	uint8_t	bin[4];
	uint8_t	data[32];
} PV_DATA;

typedef struct _CONFIG {
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
	uint8_t count_of_an_ignition;		// 15
	uint8_t sys_opt_port;				// 16

	uint8_t tp_type;					// 17
	uint8_t tp_threshold01;				// 18
	uint8_t tp_threshold02;				// 19
	uint8_t tp_threshold03;				// 20

	uint8_t sys_rpm_for_an_ignition;	// 21
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


typedef union
{
	uint8_t			rxbuf8[64];
	uint16_t		rxbuf16[32];
	status			s;
	devids			d;
	IG_DATA			ig;
	PV_DATA			pv;
	CONFIG			cfg;
} usbPacket;

#pragma pack(pop)

class HidDevice
{
private:
	uint8_t txbuf[HID_MAX_TXBUF_SIZE];
	usbPacket rxPacket;
	hid_device* dev_handle;


public:
	HidDevice(void);
	~HidDevice(void) {};

	CONFIG cfg = {};

	MsgCallback* msgfunc;

	void callback(MsgCallback* p) {msgfunc=p;}
	void msg( const char* p );

	int Open(uint16_t vid = DEVICE_VID, uint16_t pid = DEVICE_PID, wchar_t *s_number = 0);
	void Close();

	int readTable(double* pIg, double* pPv);
	int writeTable(double* pIg, double* pPv);

	int write( const uint8_t* data = nullptr, size_t length = HID_MAX_TXBUF_SIZE);
	int read( uint8_t* data = nullptr, size_t length = HID_MAX_RXBUF_SIZE);

	bool isConnected();
	bool compareRxTx();
	void checkConfig(void);

	int count_of_tune;					// Adjustment time required for the microcomputer to preprocess
	double count_of_1deg_at_1000rpm;	// Base count time which 1 degree at 1000 rpm. (16Mhz Fosc/4)
	double time2deg(uint16_t src, uint16_t rpm);
	uint16_t deg2time(double src, uint16_t rpm);

};


