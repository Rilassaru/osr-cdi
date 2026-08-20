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
12/Aug/2026 1.5.6   PVS Added a fully closed state to the initial operation of the PVS.
                    *The version has been updated to include a PCBA case for the circuit board assembly.
-----------------

Version a.b.c
        | | +Minor version up with only software change
        | +--Minor version up with hardware change
        +----Major version up
*/
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

#include "usb.h"
#include "HardwareProfile.h"
#include "userinterface.h"
#include "constant.h"

#define _XTAL_FREQ 16000000

#define mSCRGATE		LATC0	// Digital in
#define mPICKUP         RC1		// Digital in
#define mOPTPORT        RC2		// Analog in/Degital out
//      YPVS pos 		RC3		// Analog in
#define mYPVSA          LATC4	// Digital out
#define mYPVSB  		LATC5	// Digital out
//      USB+			A0
//      USB-			A1
//      USB Vbus		A2
#define mSHIFTER		RA3		// Digital in
#define mSW1			RA4		// Digital in
#define mPLUS_PU_SIGNAL_ENABLER   LATA5	// Digital out

#define CHS_THP (0b00011)       // Pin RA4/AN3
#define CHS_PVS (0b00111)       // Pin RC3/AN7

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
/** -----------------------------------------------------------------
Motor drive state
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

// Quick Shifter state
typedef enum{
	QS_IDLE = 0,
	QS_SW_ON_COUNTING,
	QS_CUT_COUNTING,
	QS_DISABLE_COUNTING,
	QS_OFF_WAITING
}QS_STATE;

// gCfg.sys_opt_port
typedef enum{
    OPT_PORT_PULSE = 0,
    OPT_PORT_MAPSW
};

// gCfg.tp_type
typedef enum{
    TP_TYPE_1CH = 0,
    TP_TYPE_2CH,
    TP_TYPE_4CH
};

#define QS_SW_ON		(0)
#define QS_SW_OFF		(1)

/** -----------------------------------------------------------------
CDI definition
------------------------------------------------------------------ */
#define NUMBER_OF_MAP_CH            (4)     // number of maps.
#define MAP_MIN_RPM                 (1)		// map able min.
#define MAP_MAX_RPM                 (159)	// map table max.
#define SCR_GATE_ON_TIME_SHORT      (20)    // Thyristor open time(us)
#define SCR_GATE_ON_TIME_LONG       (200)   // Thyristor open time(us)
#define SCR_GATE_ON_HI_AREA         (60)    // Thyristor gate time switching threshold(rpm))
#define SCR_GATE_ON                 (1)     // Thyristor gate port on.
#define SCR_GATE_OFF                (0)     // Thyristor gate port off.
#define REV_OVER_COUNT_MAX          (3)     // rev limitter, Over (n) times then cut.
#define ENGINE_STATE_STARTED        (0)     // status of engine started.
#define ENGINE_STATE_STOPPED        (1)     // status of engine stopped.
#define ENABLE_2ND_WAVE_IGNITION    (1)     // enable 2nd wave ignition
#define DISABLE_2ND_WAVE_IGNITION   (0)     // enable 2nd wave ignition

// -----------------------------------------------------------------
// To calculate the number of revolutions, choose either.
// One signal is generated per one clank rotation.
// DT200/SDR etc.. 37,500
// Two signal is generated per one clank rotation.
// RZ250/TZR250/R1-Z etc.. 18,750
// -----------------------------------------------------------------

/** -----------------------------------------------------------------
YPVS definition
------------------------------------------------------------------ */
#define PV_LEVEL_BUFFER_NUM (5) // Data size of NoiseReduction
#define NR_TRIGGER (8)  // Range to determine the electrical noise 
                        //(Decide for actual measurement) 

#define PVS_POS_MAX  (160)
#define PVS_POS_MIN (5)

#define MT_BRAKE()	{mYPVSA=1;mYPVSB=1;}
#define MT_OPEN()	{mYPVSA=1;mYPVSB=0;}
#define MT_CLOSE()	{mYPVSA=0;mYPVSB=1;}
#define MT_STOP()	{mYPVSA=0;mYPVSB=0;}

#define ENGINE_STOP_SC  (8000)  // Count to determine engine stop
#define ENGINE_START_SC (4000)  // Count to determine engine start

//------------------------------------------------------------------
//proto type
//------------------------------------------------------------------
void main(void);
void initialize_system(void);
void interrupt ISRCode();
uint8_t analog_digtal_conv8(uint8_t ch);
uint8_t pvs_leveled_analog_data(uint8_t pot);
void pvs_motor_head(uint16_t rpm, uint8_t pot);
void quick_shifter();
void initialize_pvs_min_max();

/** -----------------------------------------------------------------
 * global variable
------------------------------------------------------------------ */
uint8_t	gRPM = 0;	
uint8_t	gEngineState = ENGINE_STATE_STOPPED;
uint8_t gMapSelect = 0;             // State of map sw device. 
uint8_t	gQShifterState = QS_IDLE;  // State of Quick shifter function.
uint8_t	gQShifterCount = 0;        // Work counter for Q.S.
uint8_t gPvsMax;                   // Power valve max pos.
uint8_t gPvsMin;                   // Power valve min pos. 


/**
 * @brief Main control loop of the system.
 *
 * This function performs the following key tasks:
 * - Initializes the system and Power Valve System (PVS) settings.
 * - Monitors and updates the throttle position and related mapping.
 * - Handles USB communication and HID interface.
 * - Manages Power Valve System (PVS) motor control based on engine RPM.
 * - Updates system status and sends it to the PC.
 * 
 */
void main(void) {
    uint16_t ii;
    uint8_t anresult;
    initialize_system();
    initialize_pvs_min_max();

#ifndef __DEBUG
    // Fill PVS position data
    for(ii = 0; ii < PV_LEVEL_BUFFER_NUM; ii++) {
        GO = 1;
        while(GO);
        pvs_leveled_analog_data(analog_digtal_conv8(CHS_PVS));
        CLRWDT();
    }
#endif

    mPLUS_PU_SIGNAL_ENABLER = ENABLE_2ND_WAVE_IGNITION;

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
        pvs_motor_head(gRPM, pvs_leveled_analog_data(analog_digtal_conv8(CHS_PVS)));

        /*
         *  Throttle position convert to map page.
         */
        switch(gCfg.tp_type){
            case TP_TYPE_1CH:
                gMapSelect = 0;
                
                if(OPT_PORT_MAPSW == gCfg.sys_opt_port) {mOPTPORT = 0;}
                status.g.tp_pot = 0;
                break;

            case TP_TYPE_2CH:
                ANSELA	= 0b00000000;   // RA4(AN3) set to digtal input
                if(mSW1) {
                    gMapSelect = 1;
                    status.g.tp_pot = 0xFF;
                } else {
                    gMapSelect = 0;
                    status.g.tp_pot = 0;
                }
                
                if(OPT_PORT_MAPSW == gCfg.sys_opt_port) {mOPTPORT = gMapSelect;}
                break;

            default: // TP_TYPE_4CH
                ANSELA	= 0b00010000;   // RA4(AN3) set to analog input
                anresult = analog_digtal_conv8(CHS_THP);
                if(anresult>=gCfg.tp_threshold03)       {gMapSelect=3;}
                else if(anresult>=gCfg.tp_threshold02)  {gMapSelect=2;}
                else if(anresult>=gCfg.tp_threshold01)  {gMapSelect=1;}
                else                                    {gMapSelect=0;}
                
                // Drive map state with indicator.
                if(OPT_PORT_MAPSW == gCfg.sys_opt_port) {
                    switch(gMapSelect){
                        case 0: mOPTPORT = 0; break;
                        case 1: if(ii++>200) {ii=0;mOPTPORT=!mOPTPORT;} break;
                        case 2: if(ii++>100)  {ii=0;mOPTPORT=!mOPTPORT;} break;
                        case 3: mOPTPORT = 1; break;
                        default:mOPTPORT = 0;  break;
                    }
                }
                status.g.tp_pot = anresult;
                break;
        }
        
        /*
         * Set status report data for PC
         */
        status.g.current_map = gMapSelect;
        status.g.qs_signal = (gCfg.qs_reverse_onoff_state ? !mSHIFTER : mSHIFTER);
        status.g.qs_state = gQShifterState;
        CLRWDT();
        
    }
}


/**
 * @brief Interrupt Service Routine (ISR) for engine control.
 *
 * This function handles interrupts from various sources, including the pickup signal,
 * Timer1, and Timer2, to control the ignition timing, monitor crankshaft rotation speed,
 * and manage the engine's state. It is responsible for calculating engine RPM, setting ignition
 * timing via Timer2, handling quick shifter state, and controlling tachometer pulses.
 *
 * The function also manages engine state transitions, such as starting and stopping, 
 * and performs analog ignition handling when necessary. Additionally, it implements noise filtering
 * and rev limiter functionalities to ensure proper engine behavior.
 *
 * @note 
 * - The interrupt service routine handles engine RPM calculations, ignition delays,
 *   and quick shifter operations in response to interrupts from various timers and input signals.
 * - Timer1 is used to measure crank rotational speed, while Timer2 is used to time the ignition pulse.
 * - Timer0 is used to handle noise filtering and manage prescaler settings for the engine.
 */
void interrupt ISRCode() {
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
            tm1_count = ((TMR1H << 8)|TMR1L);

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
                mPLUS_PU_SIGNAL_ENABLER = DISABLE_2ND_WAVE_IGNITION;
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
                mPLUS_PU_SIGNAL_ENABLER = ENABLE_2ND_WAVE_IGNITION;
            }
            
            /*
             * Processing of setting to perform analog ignition for a while
             * after crank rotation starts.
             */
            if(initial_analog_ignition_count > 0) {
                TMR2ON = 0;
                TMR2IF = 0;
                mPLUS_PU_SIGNAL_ENABLER = ENABLE_2ND_WAVE_IGNITION;
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
            if(OPT_PORT_PULSE == gCfg.sys_opt_port) {mOPTPORT = 1;}
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
        mSCRGATE = SCR_GATE_OFF;
        gEngineState = ENGINE_STATE_STOPPED;
        gRPM = 0;
        gQShifterState = QS_IDLE;
        gQShifterCount = 0;

        // Pulse signal for tachometer turn off.
        if(OPT_PORT_PULSE == gCfg.sys_opt_port) {mOPTPORT = 0;}

        // Initial counter for enable 2nd wave ignition.
        initial_analog_ignition_count = gCfg.count_of_an_ignition;

        /*
         * Set the positive wave pick-up signal enabler to ignite with an analog signal.
         * A second signal will instantly turn the thyristor on.
         * However, if the analog ignition count is zero, 
         * set the pick-up signal enabler for analog ignition to off to prevent ignition.
         */
        if(0==gCfg.count_of_an_ignition) {
            mPLUS_PU_SIGNAL_ENABLER = DISABLE_2ND_WAVE_IGNITION;
        } else {
            mPLUS_PU_SIGNAL_ENABLER = ENABLE_2ND_WAVE_IGNITION;
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

        mSCRGATE = SCR_GATE_ON; // Ignite plug
        if(gRPM > SCR_GATE_ON_HI_AREA) {
            __delay_us(SCR_GATE_ON_TIME_SHORT);
        } else {
            // In the low rotation range, take a longer thyristor gate time.
            // This is a measure for early TZR and RZ.
            __delay_us(SCR_GATE_ON_TIME_LONG);
        }
        if((QS_CUT_COUNTING != gQShifterState) && (rev_over_count < REV_OVER_COUNT_MAX)) {
                mSCRGATE = SCR_GATE_OFF;
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
        if(OPT_PORT_PULSE == gCfg.sys_opt_port) {mOPTPORT = 0;}
      
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
	status.g.timer1 = gIGtbl[gMapSelect][gRPM];
	status.g.rpm    = gRPM;
	status.g.e_stop = gEngineState;
}


/**
 * @brief Quick shifter state machine for ignition cut and gear shifting.
 *
 * This function implements the state machine for a quick shifter system, which manages 
 * the ignition cutting and timing for smoother gear shifting at higher RPM. The quick 
 * shifter state machine controls the ignition cut during shifts, and handles different 
 * conditions such as switch state, RPM, and a configurable delay between shift actions.
 *
 * The state machine operates in the following states:
 * - `QS_IDLE`: Waiting for the quick shifter switch to be turned on while RPM exceeds a threshold.
 * - `QS_SW_ON_COUNTING`: Counting how long the quick shifter switch remains on before ignition cut.
 * - `QS_CUT_COUNTING`: Cutting ignition for the configured time to allow the gear shift.
 * - `QS_DISABLE_COUNTING`: Waiting to disable the quick shifter after the ignition cut.
 * - `QS_OFF_WAITING`: Waiting for the quick shifter switch to turn off after disabling.
 *
 * The function also handles an option to reverse the behavior of the quick shifter switch and 
 * to use the quick shifter port as an engine stop switch, cutting the ignition when the switch is on.
 */
void quick_shifter() {
    uint8_t shifter_sw;

    // Reverse switch option
    gCfg.qs_reverse_onoff_state ? (shifter_sw = !mSHIFTER) : (shifter_sw = mSHIFTER);
    
    // When use Quick Shifter port as an engine stop switch, 
    // Stop the ignition while the switch is on.
    if(gCfg.qs_use_as_stopsw) {
        if(QS_SW_ON == shifter_sw) {
            gQShifterState = QS_CUT_COUNTING;
        } else {
            gQShifterState = QS_IDLE;
        }
    }

    switch(gQShifterState) {
        // Waiting QS switch
        case QS_IDLE:
            if((QS_SW_ON == shifter_sw)
                    && (gRPM > gCfg.qs_enable_rpm)) {

                if(gCfg.qs_sw_on_count>0) {
                    gQShifterState = QS_SW_ON_COUNTING;
                    gQShifterCount = gCfg.qs_sw_on_count;
                } else {
                    gQShifterState = QS_CUT_COUNTING;
                    gQShifterCount = gCfg.qs_cut_count;
                }	
            }
            break;

        // Waiting ignition cut state
        case QS_SW_ON_COUNTING:
            if(QS_SW_OFF == shifter_sw) {
                gQShifterState = QS_IDLE;
            } else if((0 == gQShifterCount)) {
                gQShifterState = QS_CUT_COUNTING;
                gQShifterCount = gCfg.qs_cut_count;
            } else {
                gQShifterCount--;
            }
            break;
            
        // Ignition cutting for shift up
        case QS_CUT_COUNTING:
            if(0 == gQShifterCount) {
                gQShifterState = QS_DISABLE_COUNTING;
                gQShifterCount = gCfg.qs_disable_count;
            } else {
                gQShifterCount--;
            }
            break;

        // Disable QS switch
        case QS_DISABLE_COUNTING:
            if(0 == gQShifterCount) {
                gQShifterState = QS_OFF_WAITING;
            } else {
                gQShifterCount--;
            }
            break;

        // if the switch is broken , the switch will stay ON
        case QS_OFF_WAITING:
            if(QS_SW_OFF == shifter_sw) {
                gQShifterState = QS_IDLE;
            } // else stay this state.
            break;

        // may never in
        default:
            gQShifterState = QS_IDLE;
    }
}


/**
 * @brief Controls the Yamaha Power Valve System (YPVS) motor based on engine RPM and potentiometer position.
 *
 * This function implements the control logic for the Yamaha Power Valve System (YPVS), 
 * which adjusts the power valve position to optimize engine performance based on RPM and 
 * the potentiometer input. The system operates in multiple states to initialize and adjust 
 * the power valve position according to engine start/stop states and current engine RPM.
 *
 * The state machine controls the YPVS motor as follows:
 * - **PVS_INIT_OPEN1**:  Power valve is fully open to initialize.
 * - **PVS_INIT_CLOSE1**: Power valve is fully closed in initialize.
 * - **PVS_INIT_OPEN2**:  Power valve is fully open in initialize.
 * - **PVS_INIT_CLOSE2**: Power valve is fully closed in initialize.(optional)
 * - **PVS_ACTIVE**: Power valve adjusts dynamically based on the current RPM.
 *
 * The motor operation is divided into different states to manage the motor's movement:
 * - **MTD_OPEN**: Open the power valve.
 * - **MTD_CLOSE**: Close the power valve.
 * - **MTD_BRAKE**: Hold the valve in place.
 * - **MTD_STOP**: Stop the motor (used when reversing the motor direction).
 *
 * The function also includes error handling for analog input fluctuations and ensures smooth operation by introducing delays when reversing the motor direction.
 * 
 * @param rpm The current engine RPM. This value is clamped to the minimum and maximum allowed RPMs.
 * @param pot The current potentiometer value, which represents the current position of the power valve.
 */
void pvs_motor_head(uint16_t rpm, uint8_t pot)
{
    #define AN_VOLT_FLUCTION 2	// error of analog input.
    static uint16_t engine_mode_change_count = 0;
    static uint8_t pvs_mode = PVS_INIT_OPEN1;
    static uint8_t motor_state = MTD_BRAKE;
    uint8_t target = 0;

    if(MAP_MIN_RPM > rpm) {rpm = MAP_MIN_RPM;}
    if(rpm > MAP_MAX_RPM) {rpm = MAP_MAX_RPM;}


    /*
     *  Determine whether the engine has started or not.
     *  Only when the pickup pulse is generated consecutively, 
     *  it is judged that the engine has started.
     */
    if((ENGINE_STATE_STARTED == gEngineState) && (pvs_mode != PVS_ACTIVE)){
        if(engine_mode_change_count++ > ENGINE_START_SC) {
            pvs_mode = PVS_ACTIVE;
            engine_mode_change_count = 0;
        }
    }
    /*
     * When engine state is stopped and PVS is active, start countdown.
     * When the count expires, it judges that the engine has stopped,
     * and PVS starts initial operation.
     */
    if((ENGINE_STATE_STOPPED == gEngineState) && (pvs_mode == PVS_ACTIVE)){
        if(engine_mode_change_count++ > ENGINE_STOP_SC) {
            pvs_mode = PVS_INIT_OPEN1;
            engine_mode_change_count = 0;
        }
    }
    /*
     * YPVS state machine --
     * note:YPVS operates in the order of 
     * (1) fully open(as YPVS_INIT_OPEN1)
     * (2) fully closed(as YPVS_INIT_CLOSE)
     * (3) and fully open(as YPVS_INIT_OPEN2)
     * turning on the power supply.
     * Then, YPVS already to start engine.(YPVS_ACTIVE)
     */
    switch(pvs_mode) {
        case PVS_ACTIVE: {
            target = gPVtbl[gMapSelect][rpm];
            break;
        }	

        case PVS_INIT_OPEN1: {
            target =gPvsMax;
            if((target - AN_VOLT_FLUCTION) <= pot) {
                pvs_mode = PVS_INIT_CLOSE1;
            }
            break;
        }	

        case PVS_INIT_CLOSE1: {
            target = gPvsMin;
            if((target + AN_VOLT_FLUCTION) >= pot) {
                pvs_mode = PVS_INIT_OPEN2;
            }
            break;
        }	

        case PVS_INIT_OPEN2: {
            target = gPvsMax;
            // If the initial operation of the PVS is CLOSE, proceed to CLOSE2;
            // if it is OPEN, terminate here.
            if(1==gCfg.sys_pvs_init_pattern) {
                if((target - AN_VOLT_FLUCTION) <= pot) {
                    pvs_mode = PVS_INIT_CLOSE2;
                }
            }
            break;
        }
        
        case PVS_INIT_CLOSE2: {
            target = gPvsMin;
            break;
        }
    }


    // Motor drive
    if(target > (pot + AN_VOLT_FLUCTION)) {
        if( MTD_CLOSE == motor_state ) {
            // Blank of 100us need when reverse the motor.
            MT_STOP();
            __delay_us(120);
        }	
        motor_state = MTD_OPEN;

    } else if(target < (pot - AN_VOLT_FLUCTION)) {
        if( MTD_OPEN == motor_state ){
            MT_STOP();
            __delay_us(120);
        }	
        motor_state = MTD_CLOSE;
    } else {
        motor_state = MTD_BRAKE;
    }

	// Drive motor
    switch(motor_state) {
        case MTD_OPEN:      {MT_OPEN(); break;}
        case MTD_CLOSE:     {MT_CLOSE();break;}
        case MTD_BRAKE:     {MT_BRAKE();break;}
        case MTD_STOP:      {MT_STOP(); break;}
        default:            {MT_BRAKE();break;}
    }
    status.g.pv_pot = pot;
    status.g.pv_target = target;
    status.g.pv_mode = pvs_mode;
    status.g.pv_mt_state = motor_state;
}


/**
 * @brief Smooths and returns the leveled analog data from the potentiometer.
 *
 * This function implements a smoothing algorithm to reduce noise in the potentiometer 
 * values by maintaining a buffer of recent readings. The function computes a running 
 * average of the potentiometer values and compares the current reading with the average 
 * to detect significant changes. If the change exceeds a defined threshold (`NR_TRIGGER`), 
 * the function returns the average value to ensure a stable reading.
 *
 * @param pot The current potentiometer value to be processed.
 *
 * @return The leveled analog data:
 * - If the potentiometer value deviates significantly from the average (based on `NR_TRIGGER`), 
 *   the function returns the average of the last several readings.
 * - If the potentiometer value is within the threshold range of the average, the function returns 
 *   the current potentiometer value.
 *
 * @note The function uses a buffer (`pots`) of size `PV_LEVEL_BUFFER_NUM` to store recent readings.
 *       The average is computed over these readings to smooth the potentiometer data.
 */
uint8_t pvs_leveled_analog_data(uint8_t pot)
{
    static uint8_t pots[PV_LEVEL_BUFFER_NUM] = {0};
    uint16_t ave;
    uint16_t ii;

    ave = pots[PV_LEVEL_BUFFER_NUM-1];
    for(ii = (PV_LEVEL_BUFFER_NUM-1); ii > 0; ii--) {
        pots[ii] = pots[ii-1];
        ave += pots[ii];
    }

    ave /= PV_LEVEL_BUFFER_NUM;
    pots[0] = pot;

    if((pot > (ave+NR_TRIGGER)) || ((ave-NR_TRIGGER) > pot)) {
        return ave;
    }
    return pot;
}

/**
 * @brief Converts an analog signal to a digital value for a specified input channel.
 *
 * @param ch The analog input channel to be converted. The valid channel numbers depend on the 
 *           microcontroller's configuration and available ADC channels.
 * 
 * @return The 8-bit result of the analog-to-digital conversion. 
 */
uint8_t analog_digtal_conv8(uint8_t ch)
{
	ADCON0bits.CHS = ch;
    ADON = 1;
	__delay_us(300);    // Acquisiton delay
	GO_nDONE = 1;		// Start conversion
	while(GO_nDONE);	// Is conversion done?
	return (uint8_t)ADRESH;
    
}

/**
 * @brief Initializes the PVS minimum and maximum values based on the map data.
 *
 * This function scans the power valve table (`gPVtbl[]`) for a given range of RPM values
 * and determines the maximum and minimum values in the map. The maximum and minimum values 
 * are then used to set the initial opening and closing positions of the PVS (Power Valve System).
 *
 * The function performs the following steps:
 * - It iterates over the RPM values from `MAP_MIN_RPM` to `MAP_MAX_RPM` and checks all the map entries
 *   to find the maximum and minimum values of the PVS.
 * - The maximum value (`gPvsMax`) and the minimum value (`gPvsMin`) are updated accordingly.
 * - The function also checks for out-of-bound values and ensures that the PVS positions are within the
 *   predefined limits (`PVS_POS_MIN` and `PVS_POS_MAX`).
 *
 * These values are used to determine the initial state of the PVS, controlling its opening and closing
 * behavior based on the RPM range and the specific configuration (e.g., 1-channel, 2-channel, or 4-channel).
 *
 * @note The function is designed to work with different types of throttle position sensors (TP),
 *       as defined by `gCfg.tp_type`. It handles multiple scan modes (1-channel, 2-channel, or 4-channel)
 *       depending on the throttle type.
 */
void initialize_pvs_min_max() {
    uint8_t sw,rpm,number_of_scan;
    gPvsMax = PVS_POS_MIN;
    gPvsMin = PVS_POS_MAX;

    // From map0 to map1 and 1,00rpm to 15,900rpm,
    // scan values and get maximum and minimum values.
    switch(gCfg.tp_type){
        case TP_TYPE_1CH: {number_of_scan = 1; break;}
        case TP_TYPE_2CH: {number_of_scan = 2; break;}
        case TP_TYPE_4CH: {number_of_scan = 4; break;}
        default:          {number_of_scan = 1; break;}
    }
    
    for(sw=0; sw < number_of_scan; sw++) {
        for(rpm=MAP_MIN_RPM; rpm < MAP_MAX_RPM; rpm++) {
            if(gPvsMax < gPVtbl[sw][rpm]) {
                gPvsMax = gPVtbl[sw][rpm];
            }

            if(gPvsMin > gPVtbl[sw][rpm]) {
                gPvsMin = gPVtbl[sw][rpm];
            }
        }
    }

    // for fail safe
    if(gPvsMax>PVS_POS_MAX) {gPvsMax=PVS_POS_MAX;}
    if(gPvsMin<PVS_POS_MIN) {gPvsMin=PVS_POS_MIN;}
}



/**
 * @brief Initializes the PIC microcontroller system settings.
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
