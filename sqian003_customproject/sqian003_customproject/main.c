#include <avr/io.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include "keypad.h"
#include "io.c"
#include "timer.h"

#define SET_BIT(p,i) ((p) |= (1 << (i)))
#define CLR_BIT(p,i) ((p) &= ~(1 << (i)))
#define GET_BIT(p,i) ((p) & (1 << (i)))
#define SHIFT_REG PORTB

// create account
enum acc_States{start, createUser, userCreated, createPw, createPin, verifybotmsg, jsUD, jsLR, jsWait, finish} acc_state;
// password
enum login_States{init, enterPass, checkPass, enterPin, checkPin, resetPw, wrongWait, done} login_state;

// void login();
unsigned char cursor;

// keypad nums
unsigned char w;
unsigned char x;
unsigned char x;
unsigned char y;
unsigned char z;
unsigned char a;
unsigned char b;
unsigned char c;
unsigned char d;
unsigned char e;

// flags
unsigned char EEMEM eeprom_bot = 0;
unsigned char ram_bot = 0;

// joystick
unsigned short ud_input = 0x00;
unsigned short lr_input = 0x00;
unsigned char up_count = 0;
unsigned char down_count = 0;
unsigned char left_count = 0;
unsigned char right_count = 0;
unsigned char up_actual = 2;
unsigned char down_actual = 1;
unsigned char left_actual = 1;
unsigned char right_actual = 2;


// iterators
unsigned char h = 0;
unsigned char j = 0;
unsigned char k = 0;
unsigned char m = 0;
unsigned char n = 0;
unsigned char o = 0;
unsigned char p = 0;
unsigned char q = 0;
unsigned char r = 0;
unsigned char try = 1;

unsigned char username;
unsigned char EEMEM eeprom_user[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char EEMEM eeprom_pass[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char EEMEM eeprom_pin[4] = {0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char ram_user[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char ram_pass[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char ram_pin[4] = {0x00, 0x00, 0x00, 0x00};
	
// joystick
void A2D_init() {
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
}

void Set_A2D_Pin(unsigned char pinNum)
{
	ADMUX = (pinNum <= 0x07) ? pinNum : ADMUX;
	static unsigned char h = 0;
	for(h = 0; h < 15; ++h){asm("nop");}
}

void A2D_convert()
{
	ADCSRA |=(1<<ADSC); // begin converting
	while ( !(ADCSRA & (1<<ADIF))); // wait to make sure all pins are read
}

int main(void)
{
	/* Replace with your application code */
	DDRC = 0xF0; PORTC = 0x0F; // keypad
	DDRB = 0xFF; PORTB = 0x00; // LCD & shift register
	DDRD = 0xFF; PORTD = 0x00; // LCD
	DDRA = 0x00; PORTA = 0xFF; // joystick
	
	
	unsigned char humPattern[] = {0x0e, 0x0e, 0x04, 0x04, 0x1f, 0x04, 0x0a, 0x0a} ;
	
	TimerOn();
	TimerSet(300);
	
	transferdata(0x00); // clears lights
	
	A2D_init();
	
	LCD_init();
	LCD_WriteCommand(0x38); //Function Set: 8-bit, 2 Line, 5x7 Dots
	LCD_WriteCommand(0x01); //Clear LCD
	LCD_WriteCommand(0x06); //Entry Mode
	LCD_WriteCommand(0x0c); //Display on Cursor off
	
	ram_user[4] = eeprom_read_byte(&eeprom_user[4]);
	ram_bot = eeprom_read_byte(&eeprom_bot);
	
	
	if (ram_user[4] == 1 && ram_bot == 0) { // username made
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
	else { // no username made
		LCD_DisplayString(1, "Make Username");
	}
	
	// state = start;
	cursor = 17;
	LCD_Cursor(cursor);
	
	while (1)
	{
		while(!TimerFlag) {};
		TimerFlag = 0;
		if (ram_user[4] == 1 && ram_bot == 0) {
			login();
		}
		else {
			createAcc();
		}
	}
}

// shift register
void transferdata(unsigned char data){
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
			transferdata(0xF8);
			_delay_ms(15000);
			transferdata(0x00);
			acc_state = createPin;
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
		
		case createPin:
		LCD_DisplayString(1, "Create PIN:");
		w = GetKeypadKey();
		if (h >= 3) {
			LCD_DisplayString(1, "Pin Created");
			transferdata(0xF8);
			_delay_ms(15000);
			transferdata(0x00);
			acc_state = verifybotmsg;
			break;
		}
		else if (w != '\0') {
			LCD_Cursor(17+h);
			LCD_WriteData(w);
			eeprom_write_byte(&eeprom_pin[h], w);
			h++;
			acc_state = createPin;
		}
		break;
		
		case verifybotmsg:
		LCD_DisplayString(1, "Verify human");
		_delay_ms(15000);
		LCD_DisplayString(1, "U,D,R,U,L,R");
		acc_state = jsUD;
		
		case jsUD:
		z = GetKeypadKey();
		if (z != '#') {
			Set_A2D_Pin(0x00);
			A2D_convert();
			ud_input = ADC;
			if(ud_input > 700) {
				LCD_DisplayString(1, "up");
				++up_count;
			}
			else if (ud_input < 350) {
				LCD_DisplayString(1, "down");
				++down_count;
			}
			acc_state=jsLR;
			break;
		}
		else if (z == '#') {
			acc_state = jsWait;
			break;
		}
		
		case jsLR:
		z = GetKeypadKey();
		if (z != '#') {
			Set_A2D_Pin(0x01);
			A2D_convert();
			lr_input = ADC;
			if(lr_input > 700) {
				LCD_DisplayString(1, "left");
				++left_count;
			}
			else if (lr_input < 350) {
				LCD_DisplayString(1, "right");
				++right_count;
			}
			acc_state=jsUD;
			break;
		}
		else if (z == '#') {
			acc_state = jsWait;
			break;
		}
			
			
		case jsWait:
		if ((up_count == up_actual) && (down_count == down_actual) && (left_count == left_actual) && (right_count == right_actual)) {
			LCD_DisplayString(1, "Human Verified!");
			eeprom_write_byte(&eeprom_bot, 0);
			acc_state = finish;
			break;
		}
		else {
			LCD_DisplayString(1, "Bye Bot!");
			eeprom_write_byte(&eeprom_bot, 1);
			// LCD_ClearScreen();
			acc_state = finish;
			break;
		}
		
		case finish:
		// LCD_ClearScreen();
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
			LCD_DisplayString(1, "Password CorrectLogin Successful");
			transferdata(0xF8);
			_delay_ms(15000);
			transferdata(0x00);
			login_state = done;
			break;
		}
		else {
			try++;
			if (try < 4) {
				LCD_DisplayString(1, "Password wrong!");
				login_state = init;
				break;
			}
			if (try == 4) {
				LCD_DisplayString(1, "Enter Pin");
				login_state = enterPin;
				break;
			}
		}
		break;
		
		case enterPin:
		d = GetKeypadKey();
		if (q >= 3) {
			login_state = checkPin;
			break;
		}
		else if (d != '\0') {
			LCD_Cursor(17+q);
			LCD_WriteData('*');
			ram_pin[q] = d;
			q++;
		}
		login_state = enterPin;
		break;
		
		case checkPin:
		if ( (ram_pin[0] == eeprom_read_byte(&eeprom_pin[0])) && (ram_pin[1] == eeprom_read_byte(&eeprom_pin[1])) && (ram_pin[2] == eeprom_read_byte(&eeprom_pin[2])) ) {
			LCD_DisplayString(1, "Reset Password");
			login_state = resetPw;
			break;
		}
		else {
			LCD_DisplayString(1, "Wrong Bye!");
			_delay_ms(15000);
			LCD_ClearScreen();
			login_state = finish;
			break;
		}
		break;
		
		case resetPw:
		e = GetKeypadKey();
		if (r >= 4) {
			LCD_DisplayString(1, "Password Reset");
			transferdata(0xF8);
			_delay_ms(15000);
			transferdata(0x00);
			acc_state = done;
			break;
		}
		else if (e != '\0') {
			LCD_Cursor(17+r);
			LCD_WriteData(e);
			eeprom_update_byte(&eeprom_pass[r], e);
			r++;
			acc_state = resetPw;
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
