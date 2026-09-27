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

#include "osr_cdi.h"
#include "usb.h"
#include "pvs.h"

static uint8_t s_pvs_max_pos = 0;    // Power valve max pos.
static uint8_t s_pvs_min_pos = 0;    // Power valve min pos. 

/**
 * @brief Selects the PVS target and drives the motor toward it.
 *
 * During initialization, the target is the measured open or closed limit. Once
 * the engine is running, the target comes from the selected power-valve map.
 * Motor direction changes include a short stop interval before reversing.
 * The current position, target, mode, and motor state are copied to the USB
 * status report.
 *
 * @param rpm Engine speed used to index the power-valve map; clamped to its range.
 * @param pot Current power-valve potentiometer reading.
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
            target =s_pvs_max_pos;
            if((target - AN_VOLT_FLUCTION) <= pot) {
                pvs_mode = PVS_INIT_CLOSE1;
            }
            break;
        }	

        case PVS_INIT_CLOSE1: {
            target = s_pvs_min_pos;
            if((target + AN_VOLT_FLUCTION) >= pot) {
                pvs_mode = PVS_INIT_OPEN2;
            }
            break;
        }	

        case PVS_INIT_OPEN2: {
            target = s_pvs_max_pos;
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
            target = s_pvs_min_pos;
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
    g_status.ds.pv_pot = pot;
    g_status.ds.pv_target = target;
    g_status.ds.pv_mode = pvs_mode;
    g_status.ds.pv_mt_state = motor_state;
}


/**
 * @brief Finds the open and closed PVS limits in the configured maps.
 *
 * Scans map entries from @ref MAP_MIN_RPM up to (but not including)
 * @ref MAP_MAX_RPM. The number of maps scanned is selected by `gCfg.tp_type`.
 * The resulting limits are clamped to the supported PVS position range.
 */
void initialize_pvs_min_max(void) {
    uint8_t sw,rpm,number_of_scan;
    s_pvs_max_pos = PVS_POS_MIN;
    s_pvs_min_pos = PVS_POS_MAX;

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
            if(s_pvs_max_pos < gPVtbl[sw][rpm]) {
                s_pvs_max_pos = gPVtbl[sw][rpm];
            }

            if(s_pvs_min_pos > gPVtbl[sw][rpm]) {
                s_pvs_min_pos = gPVtbl[sw][rpm];
            }
        }
    }

    // for fail safe
    if(s_pvs_max_pos>PVS_POS_MAX) {s_pvs_max_pos=PVS_POS_MAX;}
    if(s_pvs_min_pos<PVS_POS_MIN) {s_pvs_min_pos=PVS_POS_MIN;}
}


/**
 * @brief Filters a PVS potentiometer reading against recent samples.
 *
 * Calculates the average of the previous samples. If the new reading differs
 * from that average by more than @ref NR_TRIGGER, returns the average;
 * otherwise, returns the new reading.
 *
 * @param pot Latest potentiometer reading.
 * @return Filtered reading or the latest reading when within the threshold.
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
        return (uint8_t)ave;
    }
    return pot;
}