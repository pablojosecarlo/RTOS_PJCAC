/* Copyright 2019.
 *
 * All rights reserved.
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

/* Date: 2019-05-29 */

/*==================[inclusions]=============================================*/

// Includes de FreeRTOS
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
//#include "semphr.h"     //"mutex"
//#include "timers.h"
//#include "event_groups.h"
//#include "message_buffer.h"
//#include "stream_buffer.h"
#include "sapi.h"               // <= sAPI header
#include "LCD_I2C_16F819.h"

/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/


int8_t  NUM[5] = { 0, 0, 0, 0, 0 };
uint8_t TxtBLE[9];

uint8_t stringLcd[22] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

extern QueueHandle_t myQueueLcdStringHandle;
extern QueueHandle_t myQueueLcdCommandHandle;
//extern QueueHandle_t myQueueBossHandle;


bool_t i2cWrite2(uint8_t  i2cSlaveAddress,
				 uint8_t  cmnd,
				 uint8_t  data )
{
	uint8_t cmndData[5];

	cmndData[0] = cmnd;
	cmndData[1] = data;
	cmndData[2] = 0;
	cmndData[3] = 0;
	cmndData[4] = 0;

	if ( cmnd == 0x64 && data == 0 ) return 1;

	return i2cWrite( I2C0, i2cSlaveAddress, cmndData, 5, TRUE );

}
void LCD_I2C_CLEAR(uint8_t lcd_add){
	stringLcd[0] = lcd_add;
	stringLcd[1] = 0x61;
	stringLcd[2] = 0;
	xQueueSend( myQueueLcdCommandHandle, (void*)stringLcd,  (TickType_t) 1);
};
void LCD_I2C_HOME(uint8_t lcd_add){
	stringLcd[0] = lcd_add;
	stringLcd[1] = 0x62;
	stringLcd[2] = 0;
	xQueueSend( myQueueLcdCommandHandle, (void*)stringLcd,  (TickType_t) 1);
};
void LCD_I2C_SET_ADDRESS_1234(uint8_t lcd_add, uint8_t chr_add){
	stringLcd[0] = lcd_add;
	stringLcd[1] = 0x63;
	stringLcd[2] = chr_add;
	stringLcd[3] = 0;
	xQueueSend( myQueueLcdCommandHandle, (void*)stringLcd,  (TickType_t) 1);
};
void LCD_I2C_WRITE_DATA_1234(uint8_t lcd_add, uint8_t chr_val){
	stringLcd[0] = lcd_add;
	stringLcd[1] = 0x64;
	stringLcd[2] = chr_val;
	stringLcd[3] = 0;
	xQueueSend( myQueueLcdCommandHandle, (void*)stringLcd,  (TickType_t) 1);
};
void LCD_I2C_PRUEBA(uint8_t lcd_add){
	stringLcd[0] = lcd_add;
	stringLcd[1] = 0x84;
	stringLcd[2] = 0;
	xQueueSend( myQueueLcdCommandHandle, (void*)stringLcd,  (TickType_t) 1);
};
void LCD_I2C_SETUP(uint8_t lcd_add){
	stringLcd[0] = lcd_add;
	stringLcd[1] = 0x85;
	stringLcd[2] = 0;
	xQueueSend( myQueueLcdCommandHandle, (void*)stringLcd,  (TickType_t) 1);
};

void LCD_I2C_WRITE_DATA_1234_STRING( uint8_t lcd_add, uint8_t lcd_String[]){
	stringLcd[0] = lcd_add;
	stringLcd[1] = 0x64;
	uint8_t i = 0;
	for ( i = 2; i < 22; i++ ) stringLcd[i] = 0;
	i = 2;
	while( ( i < 22) && ( lcd_String[i-2] != ';' ) && ( lcd_String[i-2] != 0 ) ){
		stringLcd[i] = lcd_String[i-2];
		i++;
	};

	xQueueSend( myQueueLcdCommandHandle, (void*)stringLcd,  (TickType_t) 1);

}

void FLOAT_A_LCD( float x){

// Envia el float con resolución +-###.## al lcd
// Prepara el arreglo TxtBLE para enviar el float como +-###.##

	FLOAT_A_DIGITAL5( x );

uint8_t  miNumero[10];
uint8_t  j = 0;

	for( j=0; j<10; j++ ){ miNumero[ j ] = 0; };
	j = 0;

	if ( x   <  0   ) { miNumero[0] = '-'; j++; };
	if ( NUM[4] > 0 ) { miNumero[j] = NUM[4] + 48; j++; };
	if ( NUM[3] > 0 ) { miNumero[j] = NUM[3] + 48; j++; };

	miNumero[j] = NUM[2] + 48; j++;
	miNumero[j] = '.'; j++;
	miNumero[j] = NUM[1] + 48; j++;
	miNumero[j] = NUM[0] + 48; j++;

	LCD_I2C_WRITE_DATA_1234_STRING( 0x66, miNumero );


	//preparo el string para mandar al BLE por la usart
	uint8_t i = 0;

	for (; i< 9; i++) TxtBLE[i] = ' ';

	i = 3;
	if (   x    < 0 ){ TxtBLE[i] = '-'        ; i++; }
	if ( NUM[4] > 0 ){ TxtBLE[i] = NUM[4] + 48; i++; }
	if ( NUM[3] > 0 ){ TxtBLE[i] = NUM[3] + 48; i++; }
	                   TxtBLE[i] = NUM[2] + 48; i++;
	                   TxtBLE[i] = '.'        ; i++;
	                   TxtBLE[i] = NUM[1] + 48; i++;

}


void FLOAT_A_DIGITAL5(float x){

// Convierte de float a dígitos ascii: Tres enteros y dos decimales
// Asume que el formato del float es ###.## i.e. se considera que solo tien dos decimales válidos
// --> solo se recuperan dos decimales !!
// La salida está en el arreglo NUM[5].

	union
	{
		unsigned char sector[1];
		unsigned int x;
	} unionX;

	float y ;
	y = x;
	if( y < 0 )	y = -y;

	unionX.x = (uint16_t) ( 100 * y );
	BIN16_A_DIGITAL5(unionX.sector[1], unionX.sector[0]);	//CONVIERTE HADD Y LADD A NUM[4] -> NUM[0]
}

void BIN16_A_DIGITAL5(uint8_t HADD, uint8_t LADD){
//Si, si. Ya sé Eric. Podria haber hecho directamente que tome un uint16_t pero pienso volver a usar este código y me gusta así

//convierte un uint_16 formando por dos mitades uint8_t en dígitos ascii que guarta en el arreglo NUM[5]

	NUM[0] = 0; NUM[1] = 0; NUM[2] = 0; NUM[3] = 0; NUM[4] = 0;

	//BLOQUE DE PASAR DE BINARIO A GRUPOS DE UNIDADES, DECENAS, CENTENAS, MILES. ETC
	if (LADD & 0b00000001){ NUM[0] +=1; }//1
	if (LADD & 0b00000010){ NUM[0] +=2; }//2
	if (LADD & 0b00000100){ NUM[0] +=4; }//4
	if (LADD & 0b00001000){ NUM[0] +=8; }//8
	if (LADD & 0b00010000){ NUM[0] +=6; NUM[1] += 1; }//16
	if (LADD & 0b00100000){ NUM[0] +=2; NUM[1] += 3; }//32
	if (LADD & 0b01000000){ NUM[0] +=4; NUM[1] += 6; }//64
	if (LADD & 0b10000000){ NUM[0] +=8; NUM[1] += 2; NUM[2] += 1; }//128

	if (HADD & 0b00000001){ NUM[0] +=6; NUM[1] += 5; NUM[2] += 2; }//256
	if (HADD & 0b00000010){ NUM[0] +=2; NUM[1] += 1; NUM[2] += 5; }//512
	if (HADD & 0b00000100){ NUM[0] +=4; NUM[1] += 2; NUM[2] += 0; NUM[3] += 1; }//1024
	if (HADD & 0b00001000){ NUM[0] +=8; NUM[1] += 4; NUM[2] += 0; NUM[3] += 2; }//2048
	if (HADD & 0b00010000){ NUM[0] +=6; NUM[1] += 9; NUM[2] += 0; NUM[3] += 4; }//4096
	if (HADD & 0b00100000){ NUM[0] +=2; NUM[1] += 9; NUM[2] += 1; NUM[3] += 8; }//8192
	if (HADD & 0b01000000){ NUM[0] +=4; NUM[1] += 8; NUM[2] += 3; NUM[3] += 6; NUM[4] += 1; }//16384
	if (HADD & 0b10000000){ NUM[0] +=8; NUM[1] += 6; NUM[2] += 7; NUM[3] += 2; NUM[4] += 3; }//32768
	//BLOQUE DE PASAR DE GRUPOS DE UNIDADES, DECENAS, CENTENAS Y MILES
	//A UNIDADES DE: UNIDAD, DECENA, CENTENA, MIL, DIEZ MIL, ETC.
	while(NUM[0] >= 0) { NUM[0] -= 10; if ( NUM[0] >= 0 ) NUM[1] += 1;} NUM[0] += 10;
	while(NUM[1] >= 0) { NUM[1] -= 10; if ( NUM[1] >= 0 ) NUM[2] += 1;} NUM[1] += 10;
	while(NUM[2] >= 0) { NUM[2] -= 10; if ( NUM[2] >= 0 ) NUM[3] += 1;} NUM[2] += 10;
	while(NUM[3] >= 0) { NUM[3] -= 10; if ( NUM[3] >= 0 ) NUM[4] += 1;} NUM[3] += 10;

}


