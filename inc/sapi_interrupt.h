/* Copyright 2019, Eric Pernia.
 * All rights reserved.
 *
 * This file is part sAPI library for microcontrollers.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/* Date: 2019/06/14 */

/*=====[Avoid multiple inclusion - begin]====================================*/

#ifndef _SAPI_INTERRUPT_H_
#define _SAPI_INTERRUPT_H_

/*=====[Inclusions of public function dependencies]==========================*/

#include "sapi_datatypes.h"
#include "sapi_gpio.h"

/*==================[c++]====================================================*/
#ifdef __cplusplus
extern "C" {
#endif

/*=====[Definitions of public data types]====================================*/
   
// interrupt events names
typedef enum{
   INTERRUPT_NO_EVENT,
   INTERRUPT_PRESSED,       // INTERRUPT_UP --> INTERRUPT_FALLING --> INTERRUPT_DOWN (falling edge)
   INTERRUPT_RELEASED,      // INTERRUPT_DOWN --> INTERRUPT_RISING --> INTERRUPT_UP (rising edge)
   INTERRUPT_HOLD_PRESED,   // INTERRUPT_PRESSED and elapsed certain time
   INTERRUPT_EVENT_HANDLED, // A previous event was handled
} interruptEvent_t;

// Button events names
typedef enum{
   INTERRUPT_ZERO_IS_UP,
   INTERRUPT_ONE_IS_UP,
} interruptLogic_t;

// FSM state names
typedef enum{
   INTERRUPT_UP,
   INTERRUPT_DOWN,
   INTERRUPT_FALLING,
   INTERRUPT_RISING
} interruptFsmState_t;

// Button object structure
typedef struct{

   // Pin and electrical connection

   int32_t gpio;
   bool_t logic;

   // Interrupt scan time

   tick_t refreshTime; // Time [ms] that refresh interrupts (update fsm)
                       //Must be more than bouncing time! i.e. 50ms

   // Interrupt FSM

   interruptFsmState_t state;
   bool_t flagUp;
   bool_t flagDown;
   bool_t flagFalling;
   bool_t flagRising;
   tick_t timeInSate; // In [ms]

   // Button event

   tick_t event;

   bool_t checkDownEvent;
   bool_t checkUpEvent;
   bool_t checkHoldDownEvent;

   tick_t holdDownTime; // In [ms]

   callBackFuncPtr_t downCallback;
   callBackFuncPtr_t upCallback;
   callBackFuncPtr_t holdDownCallback;

   void* argDownCallback;
   void* argUpCallback;
   void* argHoldDownCallback;

} interrupt_t;

/*=====[Prototypes (declarations) of public functions]=======================*/

// interrupt initialization
void interruptInit( interrupt_t* interrupt,           // interrupt structure (object)
                    int32_t gpio, bool_t logic, // Pin and electrical connection
                    tick_t refreshTime,         // interrupt scan time
                    // interrupt event
					bool_t checkDownEvent,
					bool_t checkUpEvent,
					bool_t checkHoldDownEvent,
					tick_t holdPressedTime,
					callBackFuncPtr_t downCallback,
					callBackFuncPtr_t upCallback,
					callBackFuncPtr_t holdDownCallback,
					void* argDownCallback,
					void* argUpCallback,
					void* argHoldDownCallback
);

// EVENT FUNCTIOS --------------------------------------------

// Get interrupt last event
interruptFsmState_t interruptEventGet( interrupt_t* interrupt );

// Event was handled
void interruptEventHandled( interrupt_t* interrupt );

// FSM FUNCTIOS ----------------------------------------------

// Get interrupt FSM state
interruptFsmState_t interruptStateGet( interrupt_t* interrupt );

// Get interrupt FSM time in that state
tick_t interruptTimeInStateGet( interrupt_t* interrupt );

// Init interrupt FSM
void interruptFsmInit( interrupt_t* interrupt );

// Handle FSM errors
void interruptFsmError( interrupt_t* interrupt );

// FSM Update Sate Function
void interruptFsmUpdate( interrupt_t* interrupt );

// LOW LEVEL FUNCTIONS --------------------------------------

// Return true if interrupt is up
bool_t interruptIsUp( interrupt_t* interrupt );

// Return true if interrupt is down
bool_t interruptIsDown( interrupt_t* interrupt );

/*==================[c++]====================================================*/
#ifdef __cplusplus
}
#endif

/*==================[end of file]============================================*/
#endif /* _SAPI_INTERRUPT_H_ */
