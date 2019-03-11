#include <avr/io.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include "keypad.h"
#include "io.c"
#include "timer.h"

#define SET_BIT(p,i) ((p) |= (1 << (i)))
#define CLR_BIT(p,i) ((p) &= ~(1 << (i)))
#define GET_BIT(p,i) ((p) & (1 << (i)))
#define SHIFT_REG PORTA

enum States{start, createUser, userCreated, createPw, finish} state;

// void login();
unsigned char cursor;

// keypad nums
unsigned char x;
unsigned char y;

// iterators
unsigned char j = 0;
unsigned char k = 0;
unsigned char m = 0;

unsigned char username;
unsigned char EEMEM eeprom_user[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char EEMEM eeprom_pass[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char ram_user[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char ram_pass[5] = {0x00, 0x00, 0x00, 0x00, 0x00};


int main(void)
{
	/* Replace with your application code */
	DDRC = 0xF0; PORTC = 0x0F; // keypad
	DDRB = 0xFF; PORTB = 0x00; // LCD
	DDRD = 0xFF; PORTD = 0x00; // LCD
	DDRA = 0xFF; PORTA = 0x00; // shift register
	
	unsigned char humPattern [] = {0x0e, 0x0e, 0x04, 0x04, 0x1f, 0x04, 0x0a, 0x0a} ;
	
	TimerOn();
	TimerSet(300);
	
	LCD_init();
	LCD_WriteCommand(0x38); //Function Set: 8-bit, 2 Line, 5x7 Dots
	LCD_WriteCommand(0x01); //Clear LCD
	LCD_WriteCommand(0x06); //Entry Mode
	LCD_WriteCommand(0x0c); //Display on Cursor off
	
	ram_user[4] = eeprom_read_byte(&eeprom_user[4]);
	
	if (ram_user[4] != 1) { // no username made
		LCD_DisplayString(1, "Enter Username");
	}
	else { // username made
		LCD_DisplayString(1, "Welcome  ");
		
		// creating the character
		customchar(humPattern,1);
		LCD_WriteCommand(0x88);    //Place cursor at 8th position of first row
		LCD_WriteData( 1 );
		
		
		// outputting the username
		ram_user[0] = eeprom_read_byte(&eeprom_user[0]);
		ram_user[1] = eeprom_read_byte(&eeprom_user[1]);
		ram_user[2] = eeprom_read_byte(&eeprom_user[2]);
		ram_user[3] = eeprom_read_byte(&eeprom_user[3]);
		LCD_Cursor(11);
		LCD_WriteData(ram_user[0]);
		LCD_WriteData(ram_user[1]);
		LCD_WriteData(ram_user[2]);
		LCD_WriteData(ram_user[3]);
	}
	
	state = start;
	cursor = 17;
	LCD_Cursor(cursor);
	
	while (1)
	{
		while(!TimerFlag) {};
		TimerFlag = 0;
		if (ram_user[4] != 1) {
			login();
		}
		
		
	}
}

// shift register
void transmit_data(unsigned char data){
	unsigned char inp = data;
	
	SET_BIT(SHIFT_REG, 3);	// SRCLR
	CLR_BIT(SHIFT_REG, 1);	// RCLK
	
	for(m= 0; m < 8; m++){
		CLR_BIT(SHIFT_REG, 2);  // SRCLK
		inp = GET_BIT(data, m);
		if(inp) {
			SET_BIT(SHIFT_REG, 0);	// SER
		}
		else {
			CLR_BIT(SHIFT_REG, 0); // SER
		}
		SET_BIT(SHIFT_REG, 2);	// SRCLK
	}
	SET_BIT(SHIFT_REG, 1);	// RCLK
}

// custom character
void customchar (unsigned char *Pattern, const char loc)
{
	LCD_WriteCommand(0x40+(loc*8)); // sending address
	for (int i=0; i<8; i++) {
		LCD_WriteData(Pattern[i]); // pass pattern bytes to LCD
	}
}

// login system
void login() {
	switch(state) {
		case start:
		state = createUser;
		break;
		
		case createUser:
		x = GetKeypadKey();
		if (j >= 4) {
			state = userCreated;
			break;
		}
		else if (x != '\0') {
			LCD_WriteData(x);
			eeprom_write_byte(&eeprom_user[j], x);
			j++;
		}
		state = createUser;
		break;
		
		case userCreated:
		eeprom_write_byte(&eeprom_user[4], 1);
		state = createPw;
		break;
		
		case createPw:
		LCD_DisplayString(1, "Create Password:");
		y = GetKeypadKey();
		if (k >= 4) {
			LCD_DisplayString(1, "Password Created");
			transmit_data(0xFF);
			_delay_ms(10000);
			state = finish;
			break;
		}
		else if (y != '\0') {
			LCD_Cursor(17+k);
			LCD_WriteData(y);
			eeprom_write_byte(&eeprom_pass[k], y);
			k++;
			state = createPw;
		}
		break;
		
		case finish:
		transmit_data(0x00);
		break;
	}
}