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
#include "usb.h"
#include "constant.h"

// Constants
#define CMD_GET_DATA		0x90
#define CMD_ERASE_DATA		0x91
#define CMD_SET_DATA		0x92
#define CMD_GET_STATUS		0x93
#define CMD_GET_DEVIDS		0x94
#define CMD_SET_USERIDS     0x95

#define RET_HID_CMD_SUCCESS 0x00
#define RET_HID_CMD_FAIL    0xFF

#define FLASH_PAGE_SIZE		32
#define USER_ID_BASE_ADDR   0x8000
#define USER_ID_COUNT       4

#define FW_VER_HI	(01)
#define FW_VER_LO	(56)

unsigned char ReadState;

usbPacket PacketFromPC;
usbPacket PacketToPC;
usbPacket g_status = {0};

void unlock_and_activate(void);
void send_data_at_addr(uint16_t Addr);
uint16_t read_configuration_space(uint16_t address);
uint8_t read_user_id(uint16_t *p_buf);
uint8_t write_user_id(const uint16_t *p_data);

/**
 * @brief Receives and handles one pending host command when USB is ready.
 *
 * Handles map read/write, status, and device-ID/User-ID requests. Flash and
 * configuration-space operations temporarily disable interrupts where needed.
 */
void hid_user_interface(void)
{
	uint8_t ii;
	uint16_t Addr;
    uint16_t an;


    if((USBGetDeviceState() != CONFIGURED_STATE) || (USBIsDeviceSuspended() == 1))
    {
        return;
    }
	
	if(ReadState == IDLE)
	{
		if(!mHIDRxIsBusy()) // Check if receiving some packet
		{
			HIDRxReport((char *)&PacketFromPC, USB_PACKET_SIZE);
			ReadState = NOT_IDLE;
			
			for(ii = 0; ii < USB_PACKET_SIZE; ii++)
				PacketToPC.Contents[ii] = 0;
		}
	}
	else
	{
		switch(PacketFromPC.Command)
		{
		case CMD_GET_DATA:
			if(!mHIDTxIsBusy()) {
				Addr = ADDR_IG_TABLE + (PacketFromPC.Contents[1]*FLASH_PAGE_SIZE);
				if( PacketFromPC.Contents[1] > ADDR_PAGE_MAX ) {
					PacketToPC.Contents[0] = RET_HID_CMD_FAIL;
					hid_tx_report((char *)&PacketToPC, USB_PACKET_SIZE);
				} else {
					send_data_at_addr( Addr );
				}
				ReadState = IDLE;
			}
			break;
		/*
		case CMD_ERASE_DATA:
			if(!mHIDTxIsBusy()) {
				GIE = 0;
				Addr = ADDR_IG_TABLE + (PacketFromPC.Contents[1]*FLASH_PAGE_SIZE);
				if( PacketFromPC.Contents[1] > ADDR_PAGE_MAX ) {
					PacketToPC.Contents[0] = 0xFF;
					hid_tx_report((char *)&PacketToPC, USB_PACKET_SIZE);
				}
				else
				{
					PMADR = Addr;
					PMCON1bits.CFGS = 0;
					PMCON1bits.FREE = 1;	// Enable Program Flash Erase Enable
					unlock_and_activate();

					// Make verify data
					send_data_at_addr( Addr );
				}

				ReadState = IDLE;
				GIE = 1;
			}
			break;
		*/
		case CMD_SET_DATA:
			if(!mHIDTxIsBusy()) {
				GIE = 0;			
				Addr = ADDR_IG_TABLE + (PacketFromPC.Contents[1]*FLASH_PAGE_SIZE);
				if( PacketFromPC.Contents[1] > ADDR_PAGE_MAX ) {
					PacketToPC.Contents[0] = RET_HID_CMD_FAIL;
					hid_tx_report((char *)&PacketToPC, USB_PACKET_SIZE);
				}
				else
				{
					// ----------------------------------------------------------------
					// Erase page.
					// ----------------------------------------------------------------
					PMADR = Addr;
                    PacketToPC.Contents[0] = RET_HID_CMD_SUCCESS;   // SUCCESS Flag
                    PacketToPC.Contents[1] = PMADRH;                // Address HI
                    PacketToPC.Contents[2] = PMADRL;                // Address LO
                    PacketToPC.Contents[3] = 0;                     // blank

					PMCON1bits.CFGS = 0;	// Access Flash program memory.
					PMCON1bits.FREE = 1;	// Set Program Flash Erase Enable bit 'erase'
					unlock_and_activate();
					
					// ----------------------------------------------------------------
					// Write flash.
					// ----------------------------------------------------------------
					PMCON1bits.CFGS = 0;
					PMCON1bits.FREE = 0;	// Set Program Flash Erase Enable bit 'write' 
					PMCON1bits.LWLO = 1;	// Enable Load Write Latches Only bit

					for(ii=0; ii<(FLASH_PAGE_SIZE-1); ii++) {
						PMDATL = PacketFromPC.Contents[ii+4];
						PMDATH = 0x34;	// Disable word hi byte
						unlock_and_activate();
						PMADR++;
					}

					PMCON1bits.LWLO = 0;   // Last word then write the page
					PMDATL = PacketFromPC.Contents[ii+4];	// Set last word.
					PMDATH = 0x34;	// Disable hi byte
					unlock_and_activate();

					// ----------------------------------------------------------------
					// Make verify data
					// ----------------------------------------------------------------
					send_data_at_addr( Addr );
				}

				ReadState = IDLE;
				GIE = 1;
            } //end of if(!mHIDTxIsBusy()) 
            break;

		case CMD_GET_STATUS:
            g_status.ds.cpu_temp = 0;
			if(!mHIDTxIsBusy()) {
				hid_tx_report((char *)&g_status, USB_PACKET_SIZE);
				ReadState = IDLE;
			}	
			break;

        case CMD_GET_DEVIDS:
			if(!mHIDTxIsBusy()) {
                uint16_t id;
				GIE = 0;
				// 8000h 8001h 8002h 8003h 8004h 8005h 8006h 8007h 8008h
				// <--      USERID     --> blank REVID DEVID <-Config-->
                
                // read userid
                read_user_id((uint16_t *)&PacketToPC.Contents[0]);
                
                // read revid
                id = read_configuration_space(0x8005);
				PacketToPC.di.rev_id = id;
                
                // read devid
                id = read_configuration_space(0x8006);
				PacketToPC.di.dev_id = id;
                
                // add firmware version
                PacketToPC.di.fw_ver_hi = FW_VER_HI;
                PacketToPC.di.fw_ver_lo = FW_VER_LO;
				
				hid_tx_report((char *)&PacketToPC, USB_PACKET_SIZE);
				ReadState = IDLE;
				GIE = 1;
            } //end of if(!mHIDTxIsBusy()) 
			break;
        
        case CMD_SET_USERIDS:
            if(!mHIDTxIsBusy()) {
				GIE = 0;
                uint8_t ret = write_user_id((uint16_t *)&PacketFromPC.Contents[4]);

                PacketToPC.Contents[0] = ret;
              	PacketToPC.Contents[1] = 0;
                PacketToPC.Contents[2] = 0;
                PacketToPC.Contents[3] = 0;
                read_user_id((uint16_t *)&PacketToPC.Contents[4]);
                hid_tx_report((char *)&PacketToPC, USB_PACKET_SIZE);
				ReadState = IDLE;
				GIE = 1;
            } //end of if(!mHIDTxIsBusy()) 
            break;
            
        default:
			PacketToPC.Contents[0] = RET_HID_CMD_FAIL;
			hid_tx_report((char *)&PacketToPC, USB_PACKET_SIZE);
			ReadState = IDLE;
			break;
		
		}
	}
}

/**
 * @brief Executes the PIC program-memory unlock and write sequence.
 *
 * The caller must configure the program-memory control and address registers
 * for the desired operation before calling this function.
 */
void unlock_and_activate()
{
	ClrWdt();

	// PIC16F1455/9 Revision A2 has issue of writing flash memory.
	// So, use revision A3 or later.
	// see, http://ww1.microchip.com/downloads/en/DeviceDoc/80000546F.pdf
	PMCON1bits.WREN = 1;	// Enable Program/Erase Enable bit
	PMCON2 = 0x55;		// Unlock and erase sequence
	PMCON2 = 0xAA;
	PMCON1bits.WR = 1;		// Write Control bit
	_nop();
	_nop();		// need 2 nops
	PMCON1bits.WREN = 0;	// Disable Program/Erase Enable bit

}

/**
 * @brief Resets the HID command receiver to its idle state.
 */
void user_init(void)
{
    ReadState = IDLE;
}

/**
 * @brief Reads one flash page and sends it to the host over HID.
 *
 * Interrupts are disabled during the program-memory read and restored to their
 * previous state before returning.
 *
 * @param Addr Starting program-memory address of the page to read.
 */
void send_data_at_addr(uint16_t Addr)
{
	uint8_t ii;
	PMADR = Addr;
    bool gie_state = INTCONbits.GIE;
    INTCONbits.GIE = 0;

	PacketToPC.Contents[0] = RET_HID_CMD_SUCCESS;   // SUCCESS Flag
	PacketToPC.Contents[1] = PMADRH;                // Address HI
	PacketToPC.Contents[2] = PMADRL;                // Address LO
	PacketToPC.Contents[3] = 0;                     // blank
	
	for(ii=0; ii<FLASH_PAGE_SIZE; ii++) {
		PMCON1bits.RD = 1;	// Read Control bit
        NOP(); NOP();
		PacketToPC.Contents[ii+4] = PMDATL;	// ignore HI byte
		PMADR++;
	}
	hid_tx_report((char *)&PacketToPC, USB_PACKET_SIZE);

    if (gie_state) {
        INTCONbits.GIE = 1;
    }
}

/**
 * @brief Reads one word from PIC configuration space.
 *
 * @param address Configuration-space address to read.
 * @return The 14-bit configuration word.
 */
uint16_t read_configuration_space(uint16_t address)
{
    bool gie_state = INTCONbits.GIE;
    INTCONbits.GIE = 0;

    PMADRH = (uint8_t)((address >> 8) & 0xFF);
    PMADRL = (uint8_t)(address & 0xFF);

    PMCON1bits.CFGS = 1;
    PMCON1bits.RD = 1;
    NOP(); NOP();
    PMCON1bits.CFGS = 0;

    if (gie_state) {
        INTCONbits.GIE = 1;
    }

    // Configuration Space is 14bit length.
    return (uint16_t)((((uint16_t)PMDATH << 8) | PMDATL) & 0x3FFF);
}

/**
 * @brief Reads the four PIC User ID words into the supplied array.
 *
 * @param p_buf Destination array with room for @ref USER_ID_COUNT words.
 * @return `RET_HID_CMD_SUCCESS`, or `RET_HID_CMD_FAIL` if `p_buf` is null.
 */
uint8_t read_user_id(uint16_t *p_buf)
{
    if (p_buf == NULL) {
        return RET_HID_CMD_FAIL;
    }

    bool gie_state = INTCONbits.GIE;
    INTCONbits.GIE = 0;
    PMCON1bits.CFGS = 1;

    for (uint8_t ii = 0; ii < USER_ID_COUNT; ii++) {
        uint16_t address = USER_ID_BASE_ADDR + ii;

        // Load address registers
        PMADRH = (uint8_t)((address >> 8) & 0xFF);
        PMADRL = (uint8_t)(address & 0xFF);

        // Initiate read operation
        PMCON1bits.RD = 1;
        NOP();
        NOP();

        // Store 14-bit data (mask upper 2 undefined bits)
        p_buf[ii] = (uint16_t)((((uint16_t)PMDATH << 8) | PMDATL) & 0x3FFF);
    }

    // Restore access mode back to Flash Program Memory
    PMCON1bits.CFGS = 0;
    if (gie_state) {
        INTCONbits.GIE = 1;
    }

    return RET_HID_CMD_SUCCESS;
}

/**
 * @brief Writes the four PIC User ID words from the supplied array.
 *
 * Configuration space does not support row erase, so this function writes
 * each word without setting the erase bit. Interrupt state is restored before
 * returning.
 *
 * @param p_data Source array containing @ref USER_ID_COUNT 14-bit words.
 * @return `RET_HID_CMD_SUCCESS` on success, otherwise `RET_HID_CMD_FAIL`.
 */
uint8_t write_user_id(const uint16_t *p_data)
{
    if (p_data == NULL) {
        return RET_HID_CMD_FAIL;
    }

    bool gie_state = INTCONbits.GIE;
    INTCONbits.GIE = 0;

    PMCON1bits.CFGS = 1;

    PMCON1bits.FREE = 0;
    PMCON1bits.LWLO = 0;

    for (uint8_t ii = 0; ii < USER_ID_COUNT; ii++) {
        uint16_t address = USER_ID_BASE_ADDR + ii;
        uint16_t data = p_data[ii];

        PMADRH = (uint8_t)((address >> 8) & 0xFF);
        PMADRL = (uint8_t)(address & 0xFF);
        PMDATH = (uint8_t)((data >> 8) & 0x3F);
        PMDATL = (uint8_t)(data & 0xFF);

        PMCON1bits.WREN = 1;
        PMCON2 = 0x55;
        PMCON2 = 0xAA;
        PMCON1bits.WR = 1;
        NOP();
        NOP();

        while (PMCON1bits.WR);
        PMCON1bits.WREN = 0;

        if (PMCON1bits.WRERR) {
            PMCON1bits.CFGS = 0;
            if (gie_state) {
                INTCONbits.GIE = 1;
            }
            return RET_HID_CMD_FAIL;
        }
    }

    PMCON1bits.CFGS = 0;

    if (gie_state) {
        INTCONbits.GIE = 1;
    }
    return RET_HID_CMD_SUCCESS;
}