/* Copyright 2019, Pablo josé Carlos Alonso Castillo.
 *
 * All rights reserved.
/* Date: 2019-11-29 */

/*==================[inclusions]=============================================*/

// Includes de FreeRTOS
#include "FreeRTOSConfig.h"
#include "rtos_pjcac.h"
#include "sapi_interrupt.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
//#include "timers.h"
//#include "event_groups.h"
//#include "message_buffer.h"
//#include "stream_buffer.h"

#include "LCD_I2C_16F819.h"
#include "sapi.h"               // <= sAPI header

//Defines y declaración de la Interrupciones de GPIO

	#define INTRR0  GPIO0	//Interrupcion del botón de Clear Screen del LCD
	#define INTRR1  GPIO1	//Interrupción 1 del encode incremental
	#define INTRR2  GPIO2	//Interrupción 2 del   "        "
	#define INTRR3  GPIO3	//Interrupción de lectura finalizada del sensor magnético

	#define INTERRUPT_LOGIC INTERRUPT_ONE_IS_UP

	interrupt_t myIntrr0;
	interrupt_t myIntrr1;
	interrupt_t myIntrr2;
	interrupt_t myIntrr3;

// Los argumentos de las interrupciones se deben declarar como globales.
// Las funciones de callBack son las mismas para todas las interrupciones
// y las distingo por estos argumentos
	int int0Down = 0;
	int int0Up   = 0;
	int int1Down = 1;
	int int1Up   = 1;
	int int2Down = 2;
	int int2Up   = 2;
	int int3Down = 3;
	int int3Up   = 3;


//Variables y constantes asociadas al encoder
	bool_t int1_UP = FALSE;
	bool_t int2_UP = FALSE;

	int16_t encoder_contador = 0;
	int16_t encoder_contador_previo = 0;
	int16_t encoder_contador_previo2 = 0;

	float encoder_posicion = 0;

	const float encoder_pulso_a_mm = 2.5;


//Variables asociadas al Magnetómetro
	bool_t int_magnetometro = FALSE;

	// Variables para almacenar los valores leidos del sensor
	int16_t hmc5883l_x_raw;
	int16_t hmc5883l_y_raw;
	int16_t hmc5883l_z_raw;

	// Estructura de configuracion del HMC5883L
	HMC5883L_config_t hmc5883L_configValue;


//Declaraciones y variables asociadas al FreeRTOS
SemaphoreHandle_t mySemaphoreEncoderChangeHandle = NULL;
SemaphoreHandle_t mySemaphoreMagnetometroChangeHandle =  NULL;

QueueHandle_t myQueueBossHandle = NULL;
QueueHandle_t myQueueLcdCommandHandle = NULL;

TaskHandle_t myTaskTareasLcdHandle = NULL;
TaskHandle_t myTaskInterruptHandle = NULL;
TaskHandle_t myTaskEncoderPosicionHandle = NULL;
TaskHandle_t myTaskMagnetometroLecturaHandle = NULL;
TaskHandle_t myTaskMostrarValoresLcdHandle = NULL;
TaskHandle_t myTaskIniciarLcdHandle = NULL;


// Tareas de escritura y comando del LCD
void myTaskTareasLcd(void *p){
uint8_t myRxBuffer[200];
	while( 1 ){

		gpioWrite( LED2, !gpioRead( LED2 ) ); //señal de vida. . .
		vTaskDelay( pdMS_TO_TICKS( 1000 ) );  //idem, luego lo vuelo. . .

		//Queue con los comandos y strings del lcd

		if (xQueueReceive( myQueueLcdCommandHandle, (void*)myRxBuffer, (TickType_t) 5) ){
			if ( myRxBuffer[1] != 0x64 ){

				i2cWrite2(myRxBuffer[0], myRxBuffer[1], myRxBuffer[2]);
				vTaskDelay( pdMS_TO_TICKS( 1 ) );

				//printf( "Comandos Recibidos: %s\n", myRxBuffer );

			}else{

				for (uint8_t i = 2; i < 22; i++ ){
					i2cWrite2(myRxBuffer[0], myRxBuffer[1], myRxBuffer[i]);
					vTaskDelay( pdMS_TO_TICKS( 1 ) );
				};

				//printf( "Strings Recibidos: %s\n", myRxBuffer );
			}
		}
	}
}

// Tareas de Actualización de las MEF de las interrupciones
void myTaskInterrupt(void *p){

	while( 1 ){
		gpioWrite( LED3, !gpioRead( LED3 ) );

		interruptFsmUpdate( &myIntrr0 );
		interruptFsmUpdate( &myIntrr1 );
		interruptFsmUpdate( &myIntrr2 );
		interruptFsmUpdate( &myIntrr3 );

		//vTaskDelay( pdMS_TO_TICKS( 50 ) );
	}
}

// Tareas de Actualización la posición del encoder y
// abilitacion del sensor magnético.
void myTaskEncoderPosicion(void *p){
	while( 1 ){
		if ( encoder_contador_previo != encoder_contador ){
			 encoder_contador_previo  = encoder_contador;
			 encoder_posicion = (float)encoder_contador * encoder_pulso_a_mm;
			 //printf("%2.2f", encoder_posicion);

			 xSemaphoreGive( mySemaphoreEncoderChangeHandle ); //doy el semaphoro para que el sensor magnético tome una lectura

			 //printf("posc: %i\n", encoder_contador);

		}
		vTaskDelay( pdMS_TO_TICKS( 333 ) );
	}
}

// Tareas de Lectura del nmagnetómetro y
// abilitacion de escritura en el LCD
void myTaskMagnetometroLectura(void *p){
	while( 1 ){
//		if ( int_magnetometro ){
//			 int_magnetometro = FALSE;
		if ( xSemaphoreTake( mySemaphoreEncoderChangeHandle , (TickType_t) 0xFFFFFFFF ) == 1 ){

			 hmc5883L_configValue.mode    = HMC5883L_single_measurement;
		     hmc5883lInit( hmc5883L_configValue );
			 hmc5883lRead( &hmc5883l_x_raw, &hmc5883l_y_raw, &hmc5883l_z_raw );
		}

		vTaskDelay( pdMS_TO_TICKS( 200 ) );

//		}
	}
}

// Tareas de escritura de las mediciones en el LCD
void myTaskMostrarValoresLcd(void *p){
	while( 1 ){

		if ( xSemaphoreTake( mySemaphoreMagnetometroChangeHandle , (TickType_t) 0xFFFFFFFF ) == 1 ){

			 LCD_I2C_SET_ADDRESS_1234( 0x66, 2 );
			 LCD_I2C_WRITE_DATA_1234_STRING( 0x66, "Bx = ;");
			 FLOAT_A_LCD( ( float )hmc5883l_x_raw );

			 LCD_I2C_SET_ADDRESS_1234( 0x66, 22 );
			 LCD_I2C_WRITE_DATA_1234_STRING( 0x66, "By = ;");
			 FLOAT_A_LCD( ( float )hmc5883l_y_raw );

			 LCD_I2C_SET_ADDRESS_1234( 0x66, 42 );
			 LCD_I2C_WRITE_DATA_1234_STRING( 0x66, "Bz = ;");
			 FLOAT_A_LCD( ( float )hmc5883l_z_raw );

			 LCD_I2C_SET_ADDRESS_1234( 0x66, 62 );
			 LCD_I2C_WRITE_DATA_1234_STRING( 0x66, "encoder = ;");
			 FLOAT_A_LCD( encoder_posicion );

			 //printf( "eje x: %d\r\n", hmc5883l_x_raw );
			 //printf( "eje y: %d\r\n", hmc5883l_y_raw );
			 //printf( "eje z: %d\r\n", hmc5883l_z_raw );

		}
	}
}

// Tareas de Inicialización del LCD
void myTaskIniciarLcd(void *p){

	while( 1 ){
		// Inicialización del LCD_I2C
		LCD_I2C_SETUP( 0x66 );
		LCD_I2C_CLEAR( 0x66 );
		LCD_I2C_SET_ADDRESS_1234( 0x66, 24);
		LCD_I2C_WRITE_DATA_1234_STRING( 0x66, "RTOS - PJCAC  ;");
		LCD_I2C_SET_ADDRESS_1234( 0x66, 42);
		LCD_I2C_WRITE_DATA_1234_STRING( 0x66, "10 Cohorte -2019;");
		LCD_I2C_SET_ADDRESS_1234( 0x66, 61);
		LCD_I2C_WRITE_DATA_1234_STRING( 0x66, "Encoder Magnetico;");

		//printf ("Comandos salientes: %d\n", uxQueueMessagesWaiting( myQueueLcdCommandHandle ) );

		vTaskSuspend( NULL );
	}
}

// Funciones de Callbacks para las interrupciones (Down)
void myIntrrDownCallback(void* ptr)
{
    int int_N = * (int *) ptr;

    switch ( int_N ){
    	case 0:
    		//puts("Clear LCD button pressed");
    		LCD_I2C_CLEAR( 0x66 );
    	break;
    	case 1:
    		int1_UP  = FALSE;
    	break;
    	case 2:
    		int2_UP  = FALSE;
    	break;
    	case 3:
			int_magnetometro = FALSE;
    	break;
    }
   	//printf("Intrr Down %i: \r\n", int_N);
}

// Funciones de Callbacks para las interrupciones (Up)
void myIntrrUpCallback(void* ptr)
{
    int int_N = * (int *) ptr;

    switch ( int_N ){
    	case 1:
    		int1_UP  = TRUE;
    		//solo cuento los pulsos en el int1_UP
    		if ( int2_UP) {
    			encoder_contador++;  //horario
    		}else{
    			encoder_contador--;  //antihorario
    		};
    	break;

    	case 2:
    		int2_UP  = TRUE;
    	break;

    	case 3:
    	 	//cuando pasa arriba esta lista la nueva medición
    		int_magnetometro = TRUE;
    	   	xSemaphoreGive( mySemaphoreMagnetometroChangeHandle );
    	break;
    }
	//printf("Intrr Down %i: \r\n", int_N);
}


/* FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE RESET. */
int main(void){
   /* ------------- INICIALIZACIONES ------------- */

   // Inicialización del Board de la CIAA
   boardConfig();

   // Inicialización del Sensor Magnético:

   // Cargo la Estructura de configuracion del HMC5883L con valores por defecto
   hmc5883lPrepareDefaultInit( &hmc5883L_configValue );

   // Cambio los valores que deseo configurar de la Estructura de configuracion
   // del HMC5883L y dejo el resto con sus valore por defecto
   hmc5883L_configValue.mode    = HMC5883L_idle;
   hmc5883L_configValue.samples = HMC5883L_8_sample;

   // Inicializar HMC5883L
   hmc5883lInit( hmc5883L_configValue );

   //Inicializo las interrupciones.
   //No hizo falta manejar de las interrupciones a través de FreeRTOS pues se realizan operaciones mínimas en ellas

   //Set as inputs pins used for interruptions
    gpioConfig( INTRR0, GPIO_INPUT );
    gpioConfig( INTRR1, GPIO_INPUT );
    gpioConfig( INTRR2, GPIO_INPUT );
    gpioConfig( INTRR3, GPIO_INPUT );


    // myIntrr is handled with callbacks
    interruptInit( &myIntrr0,                   // Interrupt structure (object)
 		   	   	  INTRR0, INTERRUPT_LOGIC,      // Pin and electrical connection
 				  50,                          	// Interrupt scan time [ms]
 				  TRUE,                        	// checkDownEvent
 				  TRUE,                        	// checkUpEvent
 				  FALSE,                        // checkHoldDownEvent
 				  3000,                        	// holdDownTime [ms]
 				  myIntrrDownCallback,      	// downCallback
 				  myIntrrUpCallback,     		// upCallback
 				  NULL,		 				 	// holdDownCallback
 				  &int0Down,					// argument for downCallback
 				  &int0Up,						// argument for upCallback
 				  NULL							// argument for holdDownCallback
                 );

    interruptInit( &myIntrr1,                   // Interrupt structure (object)
 		   	   	  INTRR1, INTERRUPT_LOGIC,      // Pin and electrical connection
 				  50,                          	// Interrupt scan time [ms]
 				  TRUE,                        	// checkDownEvent
 				  TRUE,                        	// checkUpEvent
 				  FALSE,                        // checkHoldDownEvent
 				  3000,                        	// holdDownTime [ms]
 				  myIntrrDownCallback,      	// downCallback
 				  myIntrrUpCallback,     		// upCallback
 				  NULL, 					 	// holdDownCallback
 				  &int1Down,					// argument for downCallback
 				  &int1Up,						// argument for upCallback
 				  NULL							// argument for holdDownCallback
                 );

    interruptInit( &myIntrr2,                   // Interrupt structure (object)
 		   	   	  INTRR2, INTERRUPT_LOGIC,      // Pin and electrical connection
 				  50,                          	// Interrupt scan time [ms]
 				  TRUE,                        	// checkDownEvent
 				  TRUE,                        	// checkUpEvent
 				  FALSE,                        // checkHoldDownEvent
 				  3000,                        	// holdDownTime [ms]
 				  myIntrrDownCallback,      	// downCallback
 				  myIntrrUpCallback,     		// upCallback
 				  NULL, 					 	// holdDownCallback
 				  &int2Down,					// argument for downCallback
 				  &int2Up,						// argument for upCallback
 				  NULL							// argument for holdDownCallback
                 );

    interruptInit( &myIntrr3,                   // Interrupt structure (object)
 		   	   	  INTRR3, INTERRUPT_LOGIC,      // Pin and electrical connection
 				  50,                          	// Interrupt scan time [ms]
 				  TRUE,                        	// checkDownEvent
 				  TRUE,                        	// checkUpEvent
 				  FALSE,                        // checkHoldDownEvent
 				  3000,                        	// holdDownTime [ms]
 				  myIntrrDownCallback,      	// downCallback
 				  myIntrrUpCallback,     		// upCallback
 				  NULL, 					 	// holdDownCallback
 				  &int3Down,					// argument for downCallback
 				  &int3Up,						// argument for upCallback
 				  NULL							// argument for holdDownCallback
                 );

    //-----------------------------------------------------------------------------------------------

    // Inicializando el i2c
    i2cInit(I2C0, 100000);

	printf("Inicializando PJCAC\r\n" );

	xTaskCreate( myTaskInterrupt, "TaskInterrupt",   configMINIMAL_STACK_SIZE*2, (void*)0, tskIDLE_PRIORITY + 1, &myTaskInterruptHandle );

	xTaskCreate( myTaskEncoderPosicion, "TaskEncoderPosicion",   configMINIMAL_STACK_SIZE*2, (void*)0, tskIDLE_PRIORITY + 1, &myTaskEncoderPosicionHandle );

	xTaskCreate( myTaskMagnetometroLectura, "myTaskMagnetometroLectura",   configMINIMAL_STACK_SIZE*2, (void*)0, tskIDLE_PRIORITY + 1, &myTaskMagnetometroLecturaHandle );

	xTaskCreate( myTaskTareasLcd,  "myTaskTareasLcd", configMINIMAL_STACK_SIZE*2, (void*)0, tskIDLE_PRIORITY + 2, &myTaskTareasLcdHandle  );

	xTaskCreate( myTaskIniciarLcd, "myTaskIniciarLcd", configMINIMAL_STACK_SIZE*2, (void*)0, tskIDLE_PRIORITY + 3, &myTaskIniciarLcdHandle );

	xTaskCreate( myTaskMostrarValoresLcd, "myTaskMostrarValoresLcd", configMINIMAL_STACK_SIZE*2, (void*)0, tskIDLE_PRIORITY + 2, &myTaskMostrarValoresLcdHandle );

	mySemaphoreEncoderChangeHandle = xSemaphoreCreateBinary();

	mySemaphoreMagnetometroChangeHandle = xSemaphoreCreateBinary();

	myQueueLcdCommandHandle = xQueueCreate( 12,  22*sizeof( uint8_t ) );

    // Iniciar scheduler
    vTaskStartScheduler();

    // Inicialización del LCD_I2C -- Se hace en las Tareas. . .

    /* ------------- REPETIR POR SIEMPRE ------------- */
    while(1){

	   //printf( "algo salió mal\n" );
	}

    /* NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa no es llamado por ningun S.O. */
    return 0 ;
}


