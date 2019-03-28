# logbot
A login system with bot verification for CS 120B custom project.
![Overall image](/Images/Overall.jpg)
## Introduction
Logbot is a login system that offers human verification. First, the user will be creating their own username, 4-digit passcode, and a PIN (for reset verification) on a keypad.To prove the user is human, he/she will then move a joystick to match the combination of joystick directions displayed on the LCD screen. If unsuccessful, a message “Bye Bot” will appear on the display, forcing the user to restart the machine to try again. Once successful, the user will then be prompted through an LCD display to enter their passcode. If it’s correct, a message will display on the LCD informing that the login was successful and LEDs will light up. If user doesn’t know his/her password, he/she has three tries. After the third try, he/she will be prompted to enter their PIN and can then reset his/her password. If he/she doesn’t know his/her PIN, he/she will have to turn the machine on/off again to get three more tries. 
<br />
<br />
**Considerations**
  - The username is 4 characters
  - The password is 4 characters
  - The PIN is 3 characters and will be used human verification
  - After completing the joystick combination, the user must press the “#” key on the keypad
  - If the joystick combination fails, the user will have to restart the system and start over to create a new username and password
  - The user must turn off and turn on the system again to be greeted with a welcome message to enter his/her password. 
  - If the user doesn’t remember his/her password, he/she only has 3 tries until he/she will be prompted to enter his/her PIN and then change the password
 ## Software
- AtmelStudio
- Programmed in C
- EEPROM
- Task 1: createAcc
![createAcc state machine](/Images/createAcc.png)
- Task 2: login
![login state machine](/Images/login.png)
## Hardware
- <a href="/Images/16x2%20LCD%20Screen.jpg">16x2 LCD Screen (output)</a>
- <a href="/Images/Joystick.jpg">2-axis Joystick Controller (input)</a>
- <a href="/Images/Keypad.jpg">Keypad (input)</a>
- <a href="/Images/Shift%20Register.jpg">Shift Register (output)</a>
- 4 LED Pins (output)
- <a href="/Images/Microcontroller.jpg">ATMega1284 Microcontroller</a>
![microcontroller pinouts](/Images/Microcontroller%20Pins.png)
- IEEE Header Chip (programming)
- Breadboard
## Demo Video
<a href="https://www.youtube.com/watch?v=LTiIfV0-Vlg&t=2s" target="_blank">Watch Demonstration</a>

