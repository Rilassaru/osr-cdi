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
#include "qshifter.h"


/**
 * @brief Updates the quick-shifter state and ignition-cut counter.
 *
 * Applies the configured switch polarity and RPM threshold, then advances the
 * state machine. When configured as an engine stop switch, the input sets the
 * state to ignition cut or idle before the state machine is advanced.
 *
 * Called from the pickup interrupt; counter durations are measured in calls.
 */
void quick_shifter(void) {
    uint8_t shifter_sw;

    // Reverse switch option
    gCfg.qs_reverse_onoff_state ? (shifter_sw = !gpioSHIFTER) : (shifter_sw = gpioSHIFTER);
    
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
