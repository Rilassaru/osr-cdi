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
/* Updates
DATE        VERSION UPDATE
01/NOV/2015 2.2.0   RCDI PIC16F1455 version released.
10/OCT/2016 2.3.0a  Solenid driver added.
26/OCT/2016 2.3.0b  Rev limitter added.
18/FEB/2017 2.3.1   Release.
04/MAY/2017 2.3.2   For RZ250R(29L), Moved the division processing into timer2 routine.
04/JUN/2017 2.3.3   Fixed for both Single and Twin specification.
-----------------
-- A product named RCDI existed in the past, so the name was changed to OSR-CDI.
-----------------
08/AUG/2017 1.0.1   OSR-CDI version beta
01/JAN/2018 1.0.4   Minor bug fix, final release
-----------------
11/MAR/2018 1.1.0   2nd P.U. signal supplessed for DT200/TZR125/RZ125
15/APL/2018 1.1.1   2nd P.U. signal enabler for DT200/TZR125/RZ125
12/MAR/2018 1.1.2   Changeable PV value to fit DT230 LANZA
-----------------
16/JUN/2018 1.2.0   Fixed some bugs and final release
17/OCT/2019 1.2.1   Plus wave pick up singal enable timing was changed.
-----------------
05/JUN/2019 1.3.0   The control of INTF was changed in order to avoid the noise
                    at the time of spark.
15/AUG/2019 1.3.0b  Thyristor gate time changed.
-----------------
14/NOV/2020 1.4.0a  Extended the ignition refusal time using Timer0.
                    Changed the behavior of Quick Shifter. 
27/Feb/2021 1.4.0b  Suppresses ignition when the first positive wave arrives.
03/JUN/2022 1.4.0c  Fix do not stop with 'use quick shifter as stop switch'.
-----------------
06/JUN/2023 1.5.0   New version released. Supports 4 maps.
26/JUL/2023 1.5.1   Changed analog ignition mechanism.
                    Changed the scanning method for maximum and minimum values
                    in the initial operation of PVS.
25/JUL/2023 1.5.2   Analog ignition control has been revised to improve starting
                    performance.
27/JUL/2023         Fixed a bug in PV control associated with 4ch.
29/SEP/2024 1.5.3   Added option to maintain analog ignition below a certain rotation speed.
10/Feb/2025 1.5.4a  The on time of the thyristor gate is made variable.
03/Mar/2025 1.5.4b  When the analog ignition count is set to 0,
                    analog ignition is not actually performed.
08/Feb/2026 1.5.5   Minor bug-fix.
12/Aug/2026 1.5.6a  PVS Added a fully closed state to the initial operation of the PVS.
                    *The version has been updated to include a PCBA case for the circuit board assembly.
06/Oct/2026 1.5.6b  USB userinterface modified UserID/DevID/RevID
26/Oct/2026 1.5.6c  XC8 Ver. 4.0 (C99) compliance and source file modularization/organization
-----------------

Version a.b.c
        | | +Minor version up with only software change
        | +--Minor version up with hardware change
        +----Major version up
*/

// CONFIG1
#pragma config FOSC = INTOSC    // Oscillator Selection Bits (INTOSC oscillator: I/O function on CLKIN pin)
#pragma config WDTE = ON        // Watchdog Timer Enable (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable (PWRT enabled)
#pragma config MCLRE = OFF      // MCLR Pin Function Select (MCLR/VPP pin function is digital input)
#pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config BOREN = ON       // Brown-out Reset Enable (Brown-out Reset enabled)
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
#pragma config IESO = OFF       // Internal/External Switchover Mode (Internal/External Switchover Mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is disabled)

// CONFIG2
#pragma config USBLSCLK = 48MHz // USB Low SPeed Clock Selection bit (System clock expects 48 MHz, FS/LS USB CLKENs divide-by is set to 8.)
#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)
#pragma config CPUDIV = CLKDIV3 // CPU System Clock Selection Bit (CPU system clock divided by 3)
#pragma config PLLMULT = 3x     // PLL Multipler Selection Bit (3x Output Frequency Selected)
#pragma config PLLEN = ENABLED  // PLL Enable Bit (3x or 4x PLL Enabled)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config LPBOR = OFF      // Low-Power Brown Out Reset (Low-Power BOR is disabled)
#pragma config LVP = OFF        // Low-Voltage Programming Enable (High-voltage on MCLR/VPP must be used for programming)

#include "HardwareProfile.h"
#include "osr_cdi.h"
#include "usb.h"

//------------------------------------------------------------------
//prototype
//------------------------------------------------------------------
void main(void);
void initialize_system(void);
void __interrupt() ISRCode(void);
uint8_t common_analog_digtal_conv8(uint8_t ch);

/** -----------------------------------------------------------------
 * global variable
------------------------------------------------------------------ */
volatile uint8_t	gRPM = 0;	
volatile uint8_t	gEngineState = ENGINE_STATE_STOPPED;
volatile uint8_t    gMapSelect = 0;
volatile uint8_t	gQShifterState = 0;
volatile uint8_t	gQShifterCount = 0;

/**
 * @brief Initializes the firmware and runs the foreground control loop.
 *
 * Enables interrupts, services USB/HID, updates the PVS and map selection,
 * and refreshes the status data reported to the host.
 */
void main(void) {
    uint16_t ii;
    uint8_t anresult;
    initialize_system();
    initialize_pvs_min_max();

    gpioANALOG_PU_ENABLER = ENABLE_2ND_WAVE_IGNITION;

    // Initialize interrupt setting
    INTE    = 1;		// Generic Interrupt on
    INTEDG  = 1;		// Interrupt on rising edge of INT pin
    TMR1IE  = 1;		// Timer1 interrupt on
    TMR2IE  = 1;		// Timer2 interrupt on
    TMR1ON  = 1;		// Timer1 start
    PEIE    = 1;
    GIE     = 1;

    // Main loop start
    ii=0;
    while (1) {
        // USB tasks
        usb_device_tasks();
        CLRWDT();
        hid_user_interface();
        CLRWDT();

        /*
         * Power Valve System (PVS) controller         
         */
        pvs_motor_head(gRPM, pvs_leveled_analog_data(common_analog_digtal_conv8(CHS_PVS)));

        /*
         *  Throttle position convert to map page.
         */
        switch(gCfg.tp_type){
            case TP_TYPE_1CH:
                gMapSelect = 0;
                
                if(OPT_PORT_MAPSW == gCfg.sys_opt_port) {gpioOPTPORT = 0;}
                g_status.ds.tp_pot = 0;
                break;

            case TP_TYPE_2CH:
                ANSELA	= 0b00000000;   // RA4(AN3) set to digtal input
                if(gpioSW1) {
                    gMapSelect = 1;
                    g_status.ds.tp_pot = 0xFF;
                } else {
                    gMapSelect = 0;
                    g_status.ds.tp_pot = 0;
                }
                
                if(OPT_PORT_MAPSW == gCfg.sys_opt_port) {gpioOPTPORT = (gMapSelect & 0x01);}
                break;

            default: // TP_TYPE_4CH
                ANSELA	= 0b00010000;   // RA4(AN3) set to analog input
                anresult = common_analog_digtal_conv8(CHS_THP);
                if(anresult>=gCfg.tp_threshold03)       {gMapSelect=3;}
                else if(anresult>=gCfg.tp_threshold02)  {gMapSelect=2;}
                else if(anresult>=gCfg.tp_threshold01)  {gMapSelect=1;}
                else                                    {gMapSelect=0;}
                
                // Drive map state with indicator.
                if(OPT_PORT_MAPSW == gCfg.sys_opt_port) {
                    switch(gMapSelect){
                        case 0: gpioOPTPORT = 0; break;
                        case 1: if(ii++>200) {ii=0;gpioOPTPORT=!gpioOPTPORT;} break;
                        case 2: if(ii++>100)  {ii=0;gpioOPTPORT=!gpioOPTPORT;} break;
                        case 3: gpioOPTPORT = 1; break;
                        default:gpioOPTPORT = 0;  break;
                    }
                }
                g_status.ds.tp_pot = anresult;
                break;
        }
        
        /*
         * Set status report data for PC
         */
        g_status.ds.current_map = gMapSelect;
        g_status.ds.qs_signal = (gCfg.qs_reverse_onoff_state ? !gpioSHIFTER : gpioSHIFTER);
        g_status.ds.qs_state = gQShifterState;
        CLRWDT();
        
    }
}


/**
 * @brief Handles the pickup and Timer0/Timer1/Timer2 interrupt events.
 *
 * Measures engine speed from the pickup interval, schedules ignition timing,
 * drives the thyristor gate, updates engine and quick-shifter state, and applies
 * noise filtering and rev limiting. This routine runs in interrupt context.
 */
void __interrupt() ISRCode(void) {
    static uint16_t tm1_count = 0;
    static uint16_t tm2_preset = 0;
    static uint8_t  rev_over_count = 0;
    static uint8_t  initial_analog_ignition_count = 0;

    /* ------------------------------------------------------------
     * Interruption by pickup signal
     * ------------------------------------------------------------
     * When Photo-coupler catch signal of pickup pulse (minus),
     * INTF is invoked. see, circuit diagram
     */
    if(INTF) {
        // While TMR2 and TMR0 are on, do not nothing now.
        // It might be noise of pick up signal.
        if((!TMR2ON)&&(!TMR0IE)) {

            /*
             * Pickup signal of minus is raised.
             * Stop Timer1 and store 16bit value of Timer1 for counting crank
             * rotation speed.
             */
            TMR1ON = 0;
            tm1_count = (((uint16_t)TMR1H << 8)|TMR1L);

            // Reset Timer1 and restart.
            TMR1H = 0;
            TMR1L = 0;
            TMR1ON = 1;

            // avoid 0 div.
            if(0 == tm1_count) {tm1_count = 0xFFFF;}

            // Calculate crank rotation speed.
            gRPM = (uint8_t)(gCfg.sys_numerator_for_rpm / (tm1_count>>3));

            /* Set delay time to Timer2 for ignite plug, value from gIGtbl[][].
             * The TMR2 setting values are in a table on Flash memory.
             * This value is created on the PC side and stored in flash memory via USB.
             */
            if((MAP_MAX_RPM >= gRPM) && (gRPM >= MAP_MIN_RPM)) {
                tm2_preset = gIGtbl[gMapSelect][gRPM];
                // Thyristor delay timer is set to TMR2.
                PR2   = (tm2_preset & 0x00FF);
                T2CON = (tm2_preset >> 8);
                gpioANALOG_PU_ENABLER = DISABLE_2ND_WAVE_IGNITION;
            }
            
            /*
             * (1) When the engine state is stopped, this is the first revolution
             * when the engine is started. If we digitally ignite at this time,
             * kickback will occur, so we do not ignite.
             * (2) Also, if there is a setting for analog ignition below a
             * certain number of revolutions, that processing is also done here.
             */
            if((ENGINE_STATE_STOPPED == gEngineState) ||
                    (gCfg.sys_rpm_for_an_ignition > gRPM)) {
                TMR2ON = 0;
                TMR2IF = 0;
                gpioANALOG_PU_ENABLER = ENABLE_2ND_WAVE_IGNITION;
            }
            
            /*
             * Processing of setting to perform analog ignition for a while
             * after crank rotation starts.
             */
            if(initial_analog_ignition_count > 0) {
                TMR2ON = 0;
                TMR2IF = 0;
                gpioANALOG_PU_ENABLER = ENABLE_2ND_WAVE_IGNITION;
                initial_analog_ignition_count--;
            }

            // Set Engine state flag
            gEngineState = ENGINE_STATE_STARTED;

            /*
             * Code must not be added from the start of interrupt until this point.
             * Since the timer to ignition has already been set,
             * if you do something, add it below.
             */
            // Quick shifter state-machine start
            quick_shifter();

            // Pulse signal for tachometer
            if(OPT_PORT_PULSE == gCfg.sys_opt_port) {gpioOPTPORT = 1;}
        }	
        INTF = 0;   // Reset interrupt
    }


    /* ------------------------------------------------------------
     * Timer1: Measuring crank rotational speed.
     * ------------------------------------------------------------
     * Timer1 is set by pickup signal(minus).
     * Timer1 timeout means engine stop. (or very low r.p.m.)
     */
     // Timer1 timeout means engine stop. (or very low r.p.m.)
	if(TMR1IF) { // Timer1 counter overflow.
        gpioSCRGATE = SCR_GATE_OFF;
        gEngineState = ENGINE_STATE_STOPPED;
        gRPM = 0;
        gQShifterState = QS_IDLE;
        gQShifterCount = 0;

        // Pulse signal for tachometer turn off.
        if(OPT_PORT_PULSE == gCfg.sys_opt_port) {gpioOPTPORT = 0;}

        // Initial counter for enable 2nd wave ignition.
        initial_analog_ignition_count = gCfg.count_of_an_ignition;

        /*
         * Set the positive wave pick-up signal enabler to ignite with an analog signal.
         * A second signal will instantly turn the thyristor on.
         * However, if the analog ignition count is zero, 
         * set the pick-up signal enabler for analog ignition to off to prevent ignition.
         */
        if(0==gCfg.count_of_an_ignition) {
            gpioANALOG_PU_ENABLER = DISABLE_2ND_WAVE_IGNITION;
        } else {
            gpioANALOG_PU_ENABLER = ENABLE_2ND_WAVE_IGNITION;
        }
                
		TMR1IF = 0; // Reset interrupt
	}

    /* ------------------------------------------------------------
     * Timer2: Thyristor gate open to ignite plug.
     * ------------------------------------------------------------
     * Timer2 measuring ignition timing from pick up signal(minus wave).
     * Timeout means that now it is time to ignite plug.
     */
    if(TMR2IF){
        TMR2ON = 0;

        gpioSCRGATE = SCR_GATE_ON; // Ignite plug
        if(gRPM > SCR_GATE_ON_HI_AREA) {
            __delay_us(SCR_GATE_ON_TIME_SHORT);
        } else {
            // In the low rotation range, take a longer thyristor gate time.
            // This is a measure for early TZR and RZ.
            __delay_us(SCR_GATE_ON_TIME_LONG);
        }
        if((QS_CUT_COUNTING != gQShifterState) && (rev_over_count < REV_OVER_COUNT_MAX)) {
                gpioSCRGATE = SCR_GATE_OFF;
        }

        /*
         * Rev limiter is a device fitted to an internal combustion engine
         * to restrict its maximum rotational speed.
         */
        if(gRPM > gCfg.rev_limit) {
            rev_over_count++;
        } else {
            rev_over_count = 0;
        }

        // Pulse signal for tachometer turn off.
        if(OPT_PORT_PULSE == gCfg.sys_opt_port) {gpioOPTPORT = 0;}
      
        /*
         * Burning the plug makes noise for a while. This causes malfunction,
         * so ignore it for a while.
         * Timer0 started for ignore the pulse for a while
         */
        TMR0 = gIgnoreNoiseTimer[gRPM];
        TMR0IF = 0;
        TMR0IE = 1;

        
         // Reset Timer2 interrupt
        TMR2IF = 0;
        // If noise occurs, the INTF flag will be raised. If INTF is up now,
        // ignore it.
        INTF = 0;
    }

    // Timer0 timeout event
	if(TMR0IF&&TMR0IE) {
        TMR0IF = 0;
        TMR0IE = 0;
        INTF = 0;
        
        /*
         * For a single engine, the prescaler is calculated at 256.
         * Since the rotation speed is doubled with the twin engine,
         * the prescaler is also halved to 128.
         * The reason for changing at this location is to update dynamically.
         */
        if(1==gCfg.sys_pulse_per_rotation) {
            OPTION_REGbits.PS = 0b111; // Pre-scaler Rate Select 256
        } else {
            OPTION_REGbits.PS = 0b110; // Pre-scaler Rate Select 128
        }

    }

	// For usb report
	g_status.ds.timer1 = gIGtbl[gMapSelect][gRPM];
	g_status.ds.rpm    = gRPM;
	g_status.ds.e_stop = gEngineState;
}


/**
 * @brief Reads one ADC channel and returns its 8-bit conversion result.
 *
 * @param ch ADC channel number to select.
 * @return The upper eight bits of the ADC result.
 */
uint8_t common_analog_digtal_conv8(uint8_t ch)
{
	ADCON0bits.CHS = ch;
    ADON = 1;
	__delay_us(300);    // Acquisiton delay
	GO_nDONE = 1;		// Start conversion
	while(GO_nDONE);	// Is conversion done?
	return (uint8_t)ADRESH;
    
}

/**
 * @brief Configures the clock, USB, I/O pins, ADC, and hardware timers.
 */
void initialize_system(void) {
	OSCCON = 0xFC; //3x PLL enabled from 16MHz HFINTOSC (1111 1100)
	ACTCON = 0x90; //Enable active clock tuning from USB
#ifndef __DEBUG
	while (OSCSTATbits.PLLRDY == 0); //Wait for PLL ready/locked
#endif
	WDTCONbits.WDTPS = 0b01100; // 01100 = 1:131072 (217) (Interval 4s nominal)

	user_init();
	usb_device_init();
	nWPUEN	= 0;
    WPUA	= 0b00010000;       // Weak pull up on at RA4
	TRISA	= 0b00011000;       // IN:RA3/4 OUT:RA5
	ANSELA	= 0b00010000;       // RA4(AN3) set to analog input
	PORTA	= 0b00000000;       // Reset PORTA

	TRISC	= 0b00001010;       // IN:RC1/3 OUT:RC0/2/4/5
	ANSELC	= 0b00001000;       // RC3(AN7) set to analog input
	PORTC	= 0b00000000;       // Reset PORTC

	ADCON2	= 0x0;              //  No auto-conversion trigger selected
	ADCON1	= 0b00100000;       // Clock:Fosc/32, VREF+ is VDD
    ADCON0bits.CHS = 0b00111;   // AN7 Selected
    ADCON0bits.GO = 0;          // GO off
    ADCON0bits.ADON = 0;        // ADConv enabled
    
    FVRCONbits.FVREN = 1;       // Fixed Voltage Reference is enabled
    FVRCONbits.TSEN = 1;        // Temperature Indicator is enabled
    FVRCONbits.TSRNG = 1;       // VOUT = VDD - 4VT (High Range)
    
	// Timer1 setteing
	T1CONbits.TMR1CS =0;        // 00 = Timer1 clock source is instruction clock (FOSC/4)
	T1CONbits.T1CKPS =0b11;     // Timer1 Input Clock Prescale Select bits 1:8 Prescale value
	T1CONbits.T1OSCEN=0;        // Dedicated Timer1 oscillator circuit disabled
	T1CONbits.nT1SYNC=1;        // 1 = Do not synchronize external clock input
	T1CONbits.TMR1ON =0;        // Timer1 On bit

	// Timer2 setting
	T2CON	= 0;
	PR2		= 0;
	TMR2IF	= 0;

	// Timer0 setting
	OPTION_REGbits.TMR0CS = 0;	// 0 = Internal instruction cycle clock (FOSC/4)
	OPTION_REGbits.PSA = 0;		// 0 = Pre-scaler is assigned to the Timer0 module
	OPTION_REGbits.PS = 0b111;  // Pre-scaler Rate Select bits
	T0IF = 0;
	T0IE = 0;
}

/** EOF main.c ***************************************************************/
