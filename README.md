ZakyntOS — Multi-Game Arduino LCD Console

Overview
ZakyntOS is a lightweight operating system for an Arduino-based mini game console running multiple classic games on a 16x2 LCD display with navigation buttons and adjustable brightness.
It combines several micro-games — such as Flappy Bird, Snake, Rock-Paper-Scissors, Pong, Dino Run, Simon Says, and a menu interface — all controlled through simple input buttons.
The system stores high scores in EEPROM, displays animations, and offers an OS-style menu for easy navigation.

Features:

Menu Navigation to choose from multiple games

Game Library:
  Flappy Bird – Jump through pipes
  Snake – Collect fruit and grow
  RPS (Rock Paper Scissors) – Play against the bot
  Pong – Classic paddle game
  Dino – Jump over obstacles
  Simon Says – Repeat the sequence
  Scores – View stored best scores

Persistent high scores stored in EEPROM

Animated OS startup screen and other animations

Adjustable brightness (via analog control)

Custom LCD icons for each game and animation effects

Hardware Used:
  Microcontroller	Arduino Nano (can use most arduino compatibal boards)
  Display	16x2 LCD (HD44780 compatible)
  Buttons	4 directional buttons (Left, Up, Right, Down)
  2 brightness buttons and 2 capacitors for stabilizing brightness
  Powered to 5V pin via Li-pol battery
  
Pin Connections
  Pins 2, 4, 5, 6, A0, A1 (buttons use "INPUT_PULLUP") - each to one leg of a button
  Pin 3 (set as "OUTPUT") (can output analog) - V0 (on LCD)
  2 * 100 µF capacitor - + leg to V0 and - leg to GND
  16 * 2 LCD Control -> 7 - RS, 8 - E, 9 - D4, 10 - D5, 11 - D6, 12 - D7
  
  GND - battery (gnd), every other button's leg 
  
EEPROM Usage
  Each game stores its best score in EEPROM memory.
  Stored automatically upon game-over if the new score exceeds the previous best.

Setup & Upload
  Open the .ino file in the Arduino IDE.
  Connect your Arduino board via USB.
  Go to Tools → Board and select your device (e.g. Arduino Nano).
  Choose the correct COM port.
  Click Upload.
  Once uploaded, the LCD should show a startup animation and enter the Menu.

License
Open-source project for educational and hobbyist use.
Developed by ZaKyntos — Hobby Electrical & Software Engineer.
