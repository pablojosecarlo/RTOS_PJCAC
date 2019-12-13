/*
 * LCD_I2C.h
 *
 *  Created on: May 29, 2019
 *      Author: Lorena B. Zacharewicz
 */


/*=====[Evitar inclusion multiple comienzo]==================================*/

#ifndef _LCD_I2C_H_
#define _LCD_I2C_H_

/*=====[Inclusiones de dependencias de funciones publicas]===================*/

#include "sapi.h"
/*=====[C++ comienzo]========================================================*/

#ifdef __cplusplus
extern "C" {
#endif


/*=======================[Macros de definicion de constantes publicas]=========================*/

/*=======================[Macros estilo funcion publicas]======================================*/

/*=======================[Definiciones de tipos de datos publicos]=============================*/

/*=======================[Tipo de datos enumerado y otros]=====================================*/
typedef enum {
   LCD_I2C_ADDRESS_0 = 0xCC,
   LCD_I2C_ADDRESS_1 = 0xCE,
   LCD_I2C_ADDRESS_2 = 0xFF,
} LCD_I2C_address_t;

/*=======================[Prototipos de funciones del I2C]====================================*/

bool_t i2cWrite2(uint8_t  i2cSlaveAddress,
				 uint8_t  cmnd,
				 uint8_t  data );


void ejecutar_comado_SLAVE_I2C(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);

uint8_t leer_Char_de_SLAVE_I2C(uint8_t, uint8_t, uint8_t);

void LCD_I2C_CLEAR(uint8_t);

void LCD_I2C_WRITE_DATA(uint8_t, uint8_t);

void LCD_I2C_HOME(uint8_t);

void LCD_I2C_SET_ADDRESS_1234(uint8_t, uint8_t);

void LCD_I2C_WRITE_DATA_1234(uint8_t, uint8_t);

void LCD_I2C_PRUEBA( uint8_t );

void LCD_I2C_SETUP( uint8_t );

void LCD_I2C_WRITE_DATA_1234_STRING( uint8_t, uint8_t[] );

void BIN16_A_DIGITAL5(uint8_t, uint8_t);	//TOMA DOS UINT8_T HADD + LADD Y LOS CONVIERTE EN NÚMEROS UINT8_T DEL NUM_5 -> NUM_0

void FLOAT_A_DIGITAL5(float);          		//TOMA UN UINT16_T Y LOS CONVIERTE EN NÚMEROS UINT8_T DEL NUM_5 -> NUM_0

void FLOAT_A_LCD(float);					//Toma un float con dos decimales y lo saca por el lcd y genera un arreglo para enviar al Bluetooth LE


/*=====[C++ fin]=============================================================*/

#ifdef __cplusplus
}
#endif

/*=====[Evitar inclusion multiple fin]=======================================*/

#endif /* _LCD_I2C_H_ */


