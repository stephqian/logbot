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

// create account
enum acc_States{start, createUser, userCreated, createPw, verifybot, jsUD, jsLR, jsWait, finish} acc_state;
// password
enum login_States{init, enterPass, checkPass, wrongWait, done} login_state;

// void login();
unsigned char cursor;

// keypad nums
unsigned char x;
unsigned char y;
unsigned char z;
unsigned char a;
unsigned char b;
unsigned char c;

// joystick
uint16_t ud_input = 0x00;
uint16_t lr_input = 0x00;
unsigned char up_count = 0x00;
unsigned char down_count = 0x00;
unsigned char left_count = 0x00;
unsigned char right_count = 0x00;
unsigned char up_actual = 1;
unsigned char down_actual = 2;
unsigned char left_actual = 1;
unsigned char right_actual = 3;


// iterators
unsigned char j = 0;
unsigned char k = 0;
unsigned char m = 0;
unsigned char n = 0;
unsigned char o = 0;
unsigned char p = 0;
unsigned char try = 1;

unsigned char username;
unsigned char EEMEM eeprom_user[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char EEMEM eeprom_pass[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char ram_user[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char ram_pass[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
	
// joystick
void A2D_init() {
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
}


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
	
	A2D_init();
	
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
	
	// state = start;
	cursor = 17;
	LCD_Cursor(cursor);
	
	while (1)
	{
		while(!TimerFlag) {};
		TimerFlag = 0;
		if (ram_user[4] != 1) {
			createAcc();
		}
		else {
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

// create account
void createAcc() {
	switch(acc_state) {
		case start:
		acc_state = createUser;
		break;
		
		case createUser:
		x = GetKeypadKey();
		if (j >= 4) {
			acc_state = userCreated;
			break;
		}
		else if (x != '\0') {
			LCD_WriteData(x);
			eeprom_write_byte(&eeprom_user[j], x);
			j++;
		}
		acc_state = createUser;
		break;
		
		case userCreated:
		eeprom_write_byte(&eeprom_user[4], 1);
		acc_state = createPw;
		break;
		
		case createPw:
		LCD_DisplayString(1, "Create Password:");
		y = GetKeypadKey();
		if (k >= 4) {
			LCD_DisplayString(1, "Password Created");
			transmit_data(0xF8);
			_delay_ms(15000);
			transmit_data(0x00);
			acc_state = verifybot;
			break;
		}
		else if (y != '\0') {
			LCD_Cursor(17+k);
			LCD_WriteData(y);
			eeprom_write_byte(&eeprom_pass[k], y);
			k++;
			acc_state = createPw;
		}
		break;
		
		case verifybot:
		LCD_DisplayString(1, "Verify you're human");
		LCD_Cursor(17);
		LCD_DisplayString(1, "1U, 2D, 1L, 3R");
		acc_state = jsUD;
		acc_state = jsLR;
		break;
		
		case jsUD:
		ADMUX = 7;
		ud_input = ADC;
		z = GetKeypadKey();
		if (z != '#') {
			if (ud_input < 512) {
				++down_count;
			}
			else if (ud_input > 512) {
				++up_count;
			}
			acc_state = jsUD;
			break;	
		}
		else {
			acc_state = jsWait;
			break;
		}
		
		case jsLR:
		ADMUX = 6;
		lr_input = ADC;
		z = GetKeypadKey();
		if (z != '#') {
			if (lr_input < 512) {
				++left_count;
			}
			else if (lr_input > 512) {
				++right_count;
			}
			acc_state = jsLR;
			break;
		}
		else {
			acc_state = jsWait;
			break;
		}
		
		case jsWait:
		if ((up_count == up_actual) && (down_count == down_actual) && (left_count == left_actual) && (right_count == right_actual)) {
			LCD_DisplayString(1, "Human Verified!");
			acc_state = finish;
			break;
		}
		else {
			LCD_DisplayString(1, "Bot Detected! Bye!");
			// LCD_ClearScreen();
			acc_state = finish;
			break;
		}
		
		case finish:
		// LCD_ClearScreen();
		LCD_WriteData(down_count);
		break;
	}
}

void login() {
	switch(login_state) {
		case init:
		_delay_ms(15000);
		LCD_DisplayString(1, "Enter Password");
		login_state = enterPass;
		break;
		
		case enterPass:
		if (try == 1) {
			a = GetKeypadKey();
			if (n >= 4) {
				login_state = checkPass;
				break;
			}
			else if (a != '\0') {
				LCD_Cursor(17+n);
				LCD_WriteData('*');
				ram_pass[n] = a;
				n++;
			}
			login_state = enterPass;
			break;
		}
		else if (try == 2) {
			b = GetKeypadKey();
			if (o >= 4) {
				login_state = checkPass;
				break;
			}
			else if (b != '\0') {
				LCD_Cursor(17+o);
				LCD_WriteData('*');
				ram_pass[o] = b;
				o++;
			}
			login_state = enterPass;
			break;
		}
		else if (try == 3) {
			c = GetKeypadKey();
			if (p >= 4) {
				login_state = checkPass;
				break;
			}
			else if (c != '\0') {
				LCD_Cursor(17+p);
				LCD_WriteData('*');
				ram_pass[p] = c;
				p++;
			}
			login_state = enterPass;
			break;
		}
		break;
		
		case checkPass:
		if ( (ram_pass[0] == eeprom_read_byte(&eeprom_pass[0])) && (ram_pass[1] == eeprom_read_byte(&eeprom_pass[1])) && (ram_pass[2] == eeprom_read_byte(&eeprom_pass[2])) && (ram_pass[3] == eeprom_read_byte(&eeprom_pass[3])) ) {
			LCD_DisplayString(1, "Password Correct");
			transmit_data(0xF8);
			_delay_ms(15000);
			login_state = done;
			break;
		}
		else {
			LCD_DisplayString(1, "Password wrong!");
			login_state = wrongWait;
			break;	
		}
		break;
		
		case wrongWait:
		if (try < 3) {
			try++;
			login_state = init;
			break;
		}
		else {
			_delay_ms(15000);
			LCD_DisplayString(1, "Bye!");
			_delay_ms(15000);
			LCD_ClearScreen();
			try = 0;
			login_state = done;
		}
		
		case done:
		break;
		
	}
}
