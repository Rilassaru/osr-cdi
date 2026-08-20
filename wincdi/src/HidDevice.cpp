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

#include <stdio.h>

#include <string.h>
#include <stdint.h>
#include <FL/fl_ask.H>
#include "HidDevice.h"
#include "mapdata.h"

#define MAX_IG_PAGE			40		// 10page x 4map
#define MAX_PV_PAGE			20		// 5page x 4map
#define MAX_CF_PAGE			2

// Clock on 1dgree at 1,000rpm is 666.67(16Mhz Fosc/4)
#define COUNT_OF_1DEG_AT_1000RPM	666.666667

/*
T2CON :rsv.<1>|T2OUTPS<3:0>|TMR2ON|T2CKPS<1:0>
*/
void putbin(uint8_t* p);


/** @brief constructor
*/
HidDevice::HidDevice() {
	msgfunc = 0;
	dev_handle = NULL;
	// set default value
	count_of_tune = COUNT_OF_TUNE_2;
	cfg.sys_pickup_degree = PICKUP_DEG_2;
	count_of_1deg_at_1000rpm = COUNT_OF_1DEG_AT_1000RPM;

	memset(txbuf, 0, HID_MAX_TXBUF_SIZE);
	memset(&rxPacket, 0, sizeof(usbPacket));
}

/** @brief Virtual function for outputting strings for debugging purposes

	@param p Message String

	@note This function should be prepared by the caller.
*/
void HidDevice::msg( const char* p ) {
	if(msgfunc) {
		msgfunc( p );
	}
}

/** @brief Safely open HID device.

	@param vid Vendor ID to identify the HID device.
	@param pid Product ID to identify the HID device.
	@param s_number Serial number assigned to the HID device.

	@returns
		Returns 1 if a connection can be made to the specified HID, 0 if not.

	@note
*/
int HidDevice::Open(uint16_t vid, uint16_t pid, wchar_t *s_number) {
	if( dev_handle ) {
		hid_close(dev_handle);
		dev_handle = 0;
	}

	dev_handle = hid_open(vid, pid, s_number);
	if( NULL == dev_handle ) {
		return 0;
	}

	return 1;
}

/** @brief Safety close HID device.

*/
void HidDevice::Close() {
	if( dev_handle ) {
		hid_close( dev_handle );
		dev_handle = nullptr;
		return;
	}
}

/** @brief Checks whether the specified HID is connected via a cable.

	@returns
		True if connected by cable, false if not found.

	@note
*/
bool HidDevice::isConnected() {
//	cur_dev->vendor_id, cur_dev->product_id
	hid_device_info* devs;

	devs = hid_enumerate(0x0, 0x0);
	for (; devs; devs = devs->next) {
		if (DEVICE_VID == devs->vendor_id && DEVICE_PID == devs->product_id) {
			hid_free_enumeration(devs);
			return true;
		}
	}

	hid_free_enumeration(devs);

	return false;
}

/** @brief Check the upper and lower limits for some settings.

*/
void HidDevice::checkConfig(void) {
	// If the value is wrong, fill with an appropriate value
	if( 40 > cfg.rev_limit) {cfg.rev_limit = 40;}
	if( 160 < cfg.rev_limit) { cfg.rev_limit = 159; }
	if (60 < cfg.count_of_an_ignition) { cfg.count_of_an_ignition = 60; }
}

/** @brief Reads configuration and table data from the device.

	@param pIg Pointer to ignition timing tables.
	@param pPv Pointer to table of power valve positions.

	@returns
		0 if there is an error, otherwise

	@note
*/
int HidDevice::readTable(double* pIg, double* pPv) {
	int page = 0, ii = 0, rpm = 0;
	double* pData = nullptr;

	if (0 == Open()) {
		msg("Open device error.\n");
		return 0;
	}

	// --------------------------------------------------------------
	// Read config values.
	// The ignition timing table cannot be converted unless the configuration data is read first.
	// --------------------------------------------------------------
	msg("Reading Config data\n");
	uint8_t* p = (uint8_t*)&cfg;

	for (page = MAX_IG_PAGE + MAX_PV_PAGE; page < MAX_IG_PAGE + MAX_PV_PAGE + MAX_CF_PAGE; page++) {
		txbuf[0] = 0;				// Report ID
		txbuf[1] = CMD_GET_DATA;	// Command
		txbuf[2] = page;			// Set page no to command.
		txbuf[3] = 0;				// null
		putbin(&txbuf[0]);

		if ((0 > write()) || (0 > read(rxPacket.rxbuf8)) || (0 != rxPacket.rxbuf8[0])) {
			msg("Error occured on reading Config data.\n");
			fl_beep(FL_BEEP_ERROR);
			fl_alert("Error occured on reading Config data.");
			Close();
			return 0;
		}
		putbin(rxPacket.rxbuf8);

		// Reading 32 values on 1 page.
		for (ii = 0; ii<32; ii++, p++) {
			*p = rxPacket.rxbuf8[4 + ii];
		}
		msg(".");
	}
	checkConfig();
	msg("OK.\n");

	// --------------------------------------------------------------
	// Read ignition page.
	// --------------------------------------------------------------
	msg( "Reading Ignition table\n" );
	pData = pIg;
	rpm = 0;
	for( page = 0; page < MAX_IG_PAGE; page++ ) {
		txbuf[0] = 0;				// Report ID
		txbuf[1] = CMD_GET_DATA;	// Command
		txbuf[2] = page;			// Set page no to command.
		txbuf[3] = 0;				// null

		if ((0 > write()) || (0 > read(rxPacket.rxbuf8)) || (0 != rxPacket.rxbuf8[0])) {
			msg( "Error occured on reading IG data.\n" );
			fl_beep(FL_BEEP_ERROR);
			fl_alert( "Error occured on reading IG data." );
			Close();
			return 0;
		}

		// read 16 values on 1 page.
		for( ii=0; ii<16; ii++ ) {
			*pData++ = time2deg(rxPacket.rxbuf16[2 + ii], (rpm % 160));
			rpm++;
		}

		puts("TX");
		putbin(txbuf);
		puts("RX");
		putbin(rxPacket.rxbuf8);

		msg( "." );
	}
	msg( "OK.\n" );

	// --------------------------------------------------------------
	// Read power valve values.
	// --------------------------------------------------------------
	msg("Reading Power Valve table\n");
	pData = pPv;
	for( pData = pPv; page<MAX_IG_PAGE+MAX_PV_PAGE; page++ ) {
		txbuf[0] = 0;				// Report ID
		txbuf[1] = CMD_GET_DATA;	// Command
		txbuf[2] = page;			// Set page no to command.
		txbuf[3] = 0;				// null
		putbin(&txbuf[0]);

		if ((0 > write()) || (0 > read(rxPacket.rxbuf8)) || (0 != rxPacket.rxbuf8[0])) {
			msg( "Error occured on reading PV data.\n" );
			fl_beep(FL_BEEP_ERROR);
			fl_alert( "Error occured on reading PV data." );
			Close();
			return 0;
		}
		puts("TX");
		putbin(txbuf);
		puts("RX");
		putbin(rxPacket.rxbuf8);

		// Reading 32 values on 1 page.
		for( ii=0; ii<32; ii++, pData++ ) {
			*pData = rxPacket.rxbuf8[4 + ii];
		}
		msg( "." );
	}
	msg( "OK.\n" );
	Close();
	return 1;

}

/** @brief Writes configuration data and table data to a device.

	@param pIg Pointer to ignition timing tables.
	@param pPv Pointer to table of power valve positions.

	@returns
		0 if there is an error, otherwise

	@note
*/
int HidDevice::writeTable(double* pIg, double* pPv) {
	int page = 0, ii = 0, rpm = 0;
	uint16_t* pTxbuf16 = nullptr;

	if (0 == Open()) {
		msg("Open device error.\n");
		return 0;
	}

	// --------------------------------------------------------------
	// Write ignition page.
	// --------------------------------------------------------------
	msg("Writing Ignition table\n");
	memset(txbuf, 0, sizeof(txbuf));

	for( page=0; page<MAX_IG_PAGE; page++ ) {
		txbuf[0] = 0x00;			// HID ReportID, must be 0.
		txbuf[1] = CMD_SET_DATA;	// WRITE command.
		txbuf[2] = page;			// Flash page no.
		txbuf[3] = 0x00;			// Blank byte.

		pTxbuf16 = (uint16_t*)&txbuf[5]; // data started from No.5 of TX buffer.
		for( ii=0; ii<16; ii++, pTxbuf16++, pIg++, rpm++ ) {
			*pTxbuf16 = deg2time(*pIg, (rpm % 160));
		}
		if(( 0 > write() )||( 0 > read(rxPacket.rxbuf8))||(0 != rxPacket.rxbuf8[0])) {
			msg( "Error occured on writing IG data.\n" );
			fl_beep(FL_BEEP_ERROR);
			fl_alert( "Error occured on writing IG data." );
			Close();
			return 0;
		}

		puts("TX");
		putbin(txbuf);
		puts("RX");
		putbin(rxPacket.rxbuf8);

		if( !compareRxTx() ) {
			msg( "Writing IG data verify failed.\n" );
			fl_beep(FL_BEEP_ERROR);
			fl_alert( "Writing IG data verify failed." );
			Close();
			return 0;
		}
		msg( "." );
		// verify
	}
	msg( "OK\n" );
	// --------------------------------------------------------------
	// Write power valve values.
	// --------------------------------------------------------------
	msg("Writing Power Valve table\n");
	rpm = 0;

	for( ; page<MAX_IG_PAGE+MAX_PV_PAGE; page++ ) {
		txbuf[0] = 0x00;			// HID ReportID, must be 0.
		txbuf[1] = CMD_SET_DATA;	// WRITE command.
		txbuf[2] = page;			// Flash page no.
		txbuf[3] = 0x00;			// Blank byte.

		pTxbuf16 = (uint16_t*)&txbuf[5]; // data started from 5 byte of TX buffer.
		for (ii = 0; ii < 32; ii++, pPv++) {
			txbuf[5 + ii] = (uint8_t)*pPv;
		}

		if(( 0 > write() )||( 0 > read(rxPacket.rxbuf8))||(0 != rxPacket.rxbuf8[0])) {
			msg( "Error occured on writing PV data.\n" );
			fl_beep(FL_BEEP_ERROR);
			fl_alert( "Error occured on writing PV data." );
			Close();
			return 0;
		}
		
		puts("TX");
		putbin(txbuf);
		puts("RX");
		putbin(rxPacket.rxbuf8);

		if( !compareRxTx() ) {
			msg( "Writing PV data verify failed.\n" );
			fl_beep(FL_BEEP_ERROR);
			fl_alert( "Writing PV data verify failed." );
			Close();
			return 0;
		}
		msg( "." );
		// verify
	}
	msg( "OK\n" );

	// --------------------------------------------------------------
	// Write config values.
	// --------------------------------------------------------------
	msg("Writing Config data\n");
	uint8_t* p = (uint8_t*)&cfg;

	for( ; page<MAX_IG_PAGE+MAX_PV_PAGE+MAX_CF_PAGE; page++ ) {
		txbuf[0] = 0x00;			// HID ReportID, must be 0.
		txbuf[1] = CMD_SET_DATA;	// WRITE command.
		txbuf[2] = page;			// Flash page no.
		txbuf[3] = 0x00;			// Blank byte.

		pTxbuf16 = (uint16_t*)&txbuf[5]; // data started from 5 byte of TX buffer.
		for( ii=0; ii<32; ii++, p++ ) {
			txbuf[5+ii] = (uint8_t)*p;
		}
		
		if(( 0 > write() )||( 0 > read(rxPacket.rxbuf8))||(0 != rxPacket.rxbuf8[0])) {
			msg( "Error occured on writing config data.\n" );
			fl_beep(FL_BEEP_ERROR);
			fl_alert( "Error occured on writing config data." );
			Close();
			return 0;
		}

		puts("TX");
		putbin(txbuf);
		puts("RX");
		putbin(rxPacket.rxbuf8);

		if( !compareRxTx() ) {
			msg( "Writing config data verify failed.\n" );
			fl_beep(FL_BEEP_ERROR);
			fl_alert( "Writing config data verify failed." );
			Close();
			return 0;
		}
		msg( "." );
		// verify
	}
	msg( "OK\n" );

	Close();
	return 1;

}

/** @brief Wrapper function for hid_Write()

	@param data Data pointer to write device.
	@paramlength Data length to write device.

	@returns
		Number of bytes written.
	@note
*/
int HidDevice::write( const uint8_t* data, size_t length ) {
	if( nullptr == dev_handle || (length > HID_MAX_TXBUF_SIZE) ) {
		msg( "Writing fail with no device.\n" );
		Close();
		return -1;
	}
	txbuf[0] = '\0';

	if( nullptr != data ) {
		memset( txbuf, 0, HID_MAX_TXBUF_SIZE); // buf[0] is HID sequence no. always 0
		memcpy( &txbuf[1], data, length );
	}
	int ret = hid_write( dev_handle, txbuf, HID_MAX_TXBUF_SIZE );
	if( ret <= 0 ) {
		Close();
		msg( "\nhid_write() fail with no device.\n" );
	}
	return ret;
}

/** @brief Wrapper function for hid_read()

	@param data Data pointer to write device.
	@paramlength Data length to size of @data buffer.

	@returns
		Number of bytes written.
	@note
*/
int HidDevice::read( uint8_t* data, size_t length ) {
	if( nullptr == dev_handle || (length > HID_MAX_RXBUF_SIZE) ) return -1;

	memset( data,0, length );
	int ret = 0;

	ret = hid_read_timeout( dev_handle, data, length, 1000 );
	if( ret <= 0 ) {
		dev_handle = nullptr;
		msg( "\nhid_rea() Failed.\n" );
	}

	return ret;
}

/** @brief Compare the contents of received data and sent data.
	@returns Unatch returns false, otherwise trure.

	@note
		The first byte of the sent data contains the report ID,
		so the content differs by 1 byte from the received data.
*/
bool HidDevice::compareRxTx() {
	for( int ii=4; ii<HID_MAX_RXBUF_SIZE; ii++ ) {
		if (this->rxPacket.rxbuf8[ii] != this->txbuf[ii + 1])
			return false;
	}
	return true;
}

/** @brief Convert ignition timing to time. 
	The time is converted to the form of Timer2 of the PIC microcontroller.

	@param deg Ignition timing.
	@param rpm R.P.M. value required for conversion.

	@returns
		Timer value for Timer2 of the PIC microcontroller.

	@note
*/
uint16_t HidDevice::deg2time( double deg, uint16_t rpm ) {
	double pre_degree, pre_count;
	uint16_t prescaler, pre, postscaler;
	uint16_t pr2, t2con;
	uint16_t result, ii;

	if (0 == deg || 0 == rpm) {
		printf("RPM:%3d DEG:%3d PRE:%3d PR2:%3d POST:%3d WRITE:%4X\n", rpm, (int)0, 0, 0, 0, (uint16_t)0);
		return 0;
	}

	// Adjust digit
	deg *=10;
	deg = (int)deg;
	deg /= 10;

	pre_degree = cfg.sys_pickup_degree - deg;
	pre_count = ((pre_degree * count_of_1deg_at_1000rpm * 10) / rpm);
	if (pre_count > count_of_tune) {
		pre_count -= count_of_tune;
	} else {
		pre_count = 0;
	}
	// Timer2 = Prescaler * PR2 * Postscaler 
	// PR2:        {0 ... 255}
	// Prescaler:  {1:1/1:4/1:16}
	// Postscaler: {1:1 ... 1:64}

	// prescaler
	ii = (uint16_t)(pre_count/256);
	if     ((ii/ 1) < 16) {prescaler = 1; pre = 0;}
	else if((ii/ 4) < 16) {prescaler = 4; pre = 1;}
	else if((ii/16) < 16) {prescaler = 16;pre = 2;}
	else if((ii/64) < 16) {prescaler = 64;pre = 3;}
	else {
		printf("*** over flow ***\n");
		printf("RPM:%3d DEG:%3d PRE:%3d PR2:%3d POST:%3d WRITE:%4X\n", rpm, (int)pre_degree, 0, 0, 0, (uint16_t)0);
		return 0;
	}

	// postscaler
	postscaler = (uint16_t)(pre_count/256/prescaler + 1);

	// pr2
	pr2 = (uint16_t)(pre_count / prescaler / postscaler);
	t2con = ((postscaler-1) << 3) + 4  + pre;
	
	result = prescaler * pr2 * postscaler - count_of_tune;
	result = (t2con<<8) + pr2;
	printf("RPM:%3d DEG:%3d PRE:%3d PR2:%3d POST:%3d WRITE:%4X\n", rpm, (int)pre_degree, prescaler, pr2, postscaler, (uint16_t)result);
	return result;
}

/** @brief Convert the Timer2 value of the PIC microcontroller to an angle.

	@param src Timer2 value of PIC microcontroller.
	@param rpm R.P.M. value required for conversion.

	@returns
		Ignition timing.

	@note
*/
double HidDevice::time2deg( uint16_t src, uint16_t rpm ) {
	uint8_t t2outps, t2ckps, pre, post, pr2;
	uint32_t count;
	double deg;

	if (0 == src) {
		printf("RPM:%3d DEG:%3d PRE:%3d PR2:%3d POST:%3d READ:%4X\n", rpm, 0, 0, 0, 0, src);
		return 0;
	}

	pr2 = src & 0xff;
	t2outps = src >> 11 & 15;
	t2ckps = (src >> 8) & 3;

	post = t2outps + 1;
	switch( t2ckps ) {
	case 0:	pre = 1;	break;
	case 1: pre = 4;	break;
	case 2: pre = 16;	break;
	case 3: pre = 64;	break;
	default: pre = 64;	 break;// Never in
	}

	count = (pre * pr2 * post) + count_of_tune;
	if (count > 0xFFFF) {
		//return 0;
	}
	deg = cfg.sys_pickup_degree - (((count*rpm) / 10) / count_of_1deg_at_1000rpm);

	// 
	deg *=10;
	deg = (int)deg;
	deg /= 10;

	printf("RPM:%3d DEG:%3d PRE:%3d PR2:%3d POST:%3d READ:%4X\n", rpm, (int)deg, pre, pr2, post, src);

	return  deg;
}


/** @brief Outputs debugging data to the console.

	@param p pointer to data.

	@note
		Outputs a total of 64 times.
*/
void putbin(uint8_t* p) {
	int ii, jj;
	puts("00-01-02-03-04-05-06-07-08=09-0A-0B-0C-0D-0E-0F");
	for(ii = 0; ii < 4; ii++) {
		for(jj = 0; jj < 16; jj++) {
			printf( "%02X ", (uint8_t)p[ii*16+jj]);
		}
		for(jj = 0; jj < 16; jj++) {
			if( p[ii*16+jj] >= 0x20 && p[ii*16+jj] <=0x7e )
				printf( "%c", p[ii*16+jj]);
			else
				printf( "." );
		}
		

		puts( "" );
	}
}

