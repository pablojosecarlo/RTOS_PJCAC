/*=============================================================================
 * Copyright (c) 2019, Eric Pernia <ericpernia@gmail.com>
 * All rights reserved.
 * License: bsd-3-clause (see LICENSE file)
 * Date: 2019/06/14
 * Version: 1.0.0
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include "sapi_interrupt.h"

/*=====[Private functions declarations]======================================*/

static interruptFsmState_t interruptInitState( interrupt_t* interrupt );

/*=====[Public functions definitions]========================================*/

// Button initialization
void interruptInit( interrupt_t* interrupt,        // interrupt structure (object)
                 	int32_t gpio, bool_t logic, // Pin and electrical connection
					tick_t refreshTime,         // interrupt scan time
					// Interrupt events
					bool_t checkDownEvent,
					bool_t checkUpEvent,
					bool_t checkHoldDownEvent,
					tick_t holdDownTime,
					callBackFuncPtr_t downCallback,
					callBackFuncPtr_t upCallback,
					callBackFuncPtr_t holdDownCallback,
					void* argDownCallback,
					void* argUpCallback,
					void* argHoldDownCallback
				  )
{
   // Pin and electrical connection
   interrupt->gpio = gpio;
   interrupt->logic = logic;

   // Interrupt scan time
   interrupt->refreshTime = refreshTime;

   // Interrupt FSM
   interruptFsmInit( interrupt );

   // Interrupt events
   interrupt->event = INTERRUPT_NO_EVENT;

   interrupt->checkDownEvent = checkDownEvent;
   interrupt->checkUpEvent = checkUpEvent;
   interrupt->checkHoldDownEvent = checkHoldDownEvent;

   interrupt->holdDownTime = holdDownTime;

   interrupt->downCallback = downCallback;
   interrupt->upCallback = upCallback;
   interrupt->holdDownCallback = holdDownCallback;

   interrupt->argDownCallback = argDownCallback;
   interrupt->argUpCallback = argUpCallback;
   interrupt->argHoldDownCallback = argHoldDownCallback;

}

// EVENT FUNCTIOS --------------------------------------------

// Get Interrupt last event
interruptFsmState_t interruptEventGet( interrupt_t* interrupt )
{
   return interrupt->event;
}

// Event was handled
void interruptEventHandled( interrupt_t* interrupt )
{
	interrupt->event = INTERRUPT_EVENT_HANDLED;
}

// FSM FUNCTIOS ----------------------------------------------

// Get interrupt FSM state
interruptFsmState_t interruptStateGet( interrupt_t* interrupt )
{
   return interrupt->state;
}

// Get interrupt FSM time in that state
tick_t interruptTimeInStateGet( interrupt_t* interrupt )
{
   return interrupt->timeInSate;
}

// Init interrupt FSM
void interruptFsmInit( interrupt_t* interrupt )
{
	interrupt->state = interruptInitState( interrupt );
	interrupt->timeInSate = 0;
	interrupt->flagUp = FALSE;
	interrupt->flagDown = FALSE;
	interrupt->flagFalling = FALSE;
	interrupt->flagRising = FALSE;
}

// Handle FSM errors
void interruptFsmError( interrupt_t* interrupt )
{
   // TODO
}

// FSM Update Sate Function
void interruptFsmUpdate( interrupt_t* interrupt )
{   
   interrupt->timeInSate += interrupt->refreshTime;
   switch( interrupt->state ){

      case INTERRUPT_UP:
         // ENTRY
         if( interrupt->flagUp == FALSE ){
        	 interrupt->flagUp = TRUE;
        	 interrupt->event = INTERRUPT_NO_EVENT;
         }
         // CHECK TRANSITION CONDITIONS
         if( interruptIsDown(interrupt) ){
        	 interrupt->state = INTERRUPT_FALLING;
        	 interrupt->timeInSate = 0;
         }
         // EXIT
         if( interrupt->state != INTERRUPT_UP ){
        	 interrupt->flagUp = FALSE;
        	 interrupt->timeInSate = 0;
        	 interrupt->event = INTERRUPT_NO_EVENT;
         }
      break;

      case INTERRUPT_DOWN:
         // ENTRY
         if( interrupt->flagDown == FALSE ){
        	 interrupt->flagDown = TRUE;
        	 interrupt->event = INTERRUPT_NO_EVENT;
         }
         // CHECK TRANSITION CONDITIONS
         if( interruptIsUp(interrupt) ){
        	 interrupt->state = INTERRUPT_RISING;
         }
         // Check and execute if correspond hold down event
         if( (interrupt->checkHoldDownEvent) &&
             (interrupt->timeInSate > interrupt->holdDownTime) ){
            if( interrupt->event == INTERRUPT_NO_EVENT ){
            	interrupt->event = INTERRUPT_HOLD_PRESED;
               // Execute callback function if check event and pointer is not NULL
               if( (interrupt->holdDownCallback != NULL) ){
                  (*(interrupt->holdDownCallback))( interrupt->argHoldDownCallback );
               }
            }
         }
         // EXIT
         if( interrupt->state != INTERRUPT_DOWN ){
        	 interrupt->flagDown = FALSE;
        	 interrupt->timeInSate = 0;
        	 interrupt->event = INTERRUPT_NO_EVENT;
         }
      break;

      case INTERRUPT_FALLING:
         // ENTRY
         if( interrupt->flagFalling == FALSE ){
        	 interrupt->flagFalling = TRUE;
            //gpioWrite(DO6, ON);
         }      
         // CHECK TRANSITION CONDITIONS
         if( interruptIsDown(interrupt) ){
        	 interrupt->state = INTERRUPT_DOWN;
            // Check and execute if correspond down event
            if( (interrupt->checkDownEvent) ){
            	interrupt->event = INTERRUPT_PRESSED;
               // Execute callback function if check event and pointer is not NULL
               if( (interrupt->downCallback != NULL) ){
                  (*(interrupt->downCallback))( interrupt->argDownCallback );
               }
            }
         } else{
        	 interrupt->state = INTERRUPT_UP;
         }         
         // EXIT
         if( interrupt->state != INTERRUPT_FALLING ){
        	 interrupt->flagFalling = FALSE;
        	 interrupt->timeInSate = 0;
            //gpioWrite(DO6, OFF);
         }
      break;

      case INTERRUPT_RISING:
         // ENTRY
         if( interrupt->flagRising == FALSE ){
        	 interrupt->flagRising = TRUE;
            //gpioWrite(DO7, ON);
         }    
         // CHECK TRANSITION CONDITIONS
         if( interruptIsUp(interrupt) ){
        	 interrupt->state = INTERRUPT_UP;
            // Check and execute if correspond up event
            if( (interrupt->checkUpEvent) ){
            	interrupt->event = INTERRUPT_RELEASED;
               // Execute callback function if check event and pointer is not NULL
               if( (interrupt->upCallback != NULL) ){
                  (*(interrupt->upCallback))( interrupt->argUpCallback );
               }
            }
         } else{
        	 interrupt->state = INTERRUPT_DOWN;
         }

         // EXIT
         if( interrupt->state != INTERRUPT_RISING ){
        	 interrupt->flagRising = FALSE;
        	 interrupt->timeInSate = 0;
            //gpioWrite(DO7, OFF);
         }
      break;

      default:
         interruptFsmError(interrupt);
      break;
   }
}

// LOW LEVEL FUNCTIONS --------------------------------------

// Return true if interrupt is up
bool_t interruptIsUp( interrupt_t* interrupt )
{
   bool_t gpioStatus = gpioRead( interrupt->gpio );
   if( interrupt->logic == INTERRUPT_ONE_IS_UP ){
      return gpioStatus;
   } else{
      return !gpioStatus;
   }
}

// Return true if interrupt is down
bool_t interruptIsDown( interrupt_t* interrupt )
{
   return !interruptIsUp( interrupt );
}

/*=====[Private functions definitions]=======================================*/

static interruptFsmState_t interruptInitState( interrupt_t* interrupt )
{
   if( interruptIsDown(interrupt) ){
      return INTERRUPT_DOWN;
   }else{
      return INTERRUPT_UP;
   }
}
