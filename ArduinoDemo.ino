//Arduino Uno with Matrix Orbital ELK204-7T-1U (I2C Mode) and modified BBPDC
//Created by Clark, 21/09/2012
//support@matrixorbital.ca 
//www.matrixorbital.ca/appnotes

#include <Wire.h>
#include <stdlib.h>

#define I2C_Address 0x28  //Define default 8bit I2C address of 0x50 >> 1 for 7bit Arduino

//global variables
byte current_selection = 1;
byte previous_selection = 4;
int selected_colour = 0;
byte backlight_colours[8][3] = {{255,255,255},{255,0,0},{0,255,0},{0,0,255},{0,255,255},{208,228,0},{255,0,255},{0,255,85}};
int contrast = 128;
int selected_led = 1;
int led_state = 0;

void setup(){
  //Setup I2C bus and serial reporting
  Wire.begin();
  Serial.begin(19200);
  //Setup display
  setReportingMode(0);  //Transmit via i2c port rather than serial
  autoTxKeysOff();  //Hold key presses in output buffer to be polled
  reset();
  drawMainMenu();
  updateMenu(0);
}

void loop(){  //Main  menu loop
  switch(readKey()){
    case 66:  //up
      updateMenu(-1);
      break;
    case 72:  //down
      updateMenu(1);
      break;
    case 69:  //centre
      gotoSubMenu(current_selection);
      drawMainMenu();
      updateMenu(0);
      break;
  }
  delay(100);
}

void drawMainMenu(){
  char* menu_strings[]={"Backlight Colour","Contrast Setting","LED Indicators","Reset to Defaults"};
  clearScreen();
  for(int i = 0; i < 4; i++){
    writeMessage(menu_strings[i], strlen(menu_strings[i]), 2, i+1);
  }
}

void drawMenuCursor(int last, int present){
  char blank[] = { 0x20 };
  char arrow[] = { 0x7E };
  writeMessage(blank, 1, 1, last);
  writeMessage(arrow, 1, 1, present);
}

void updateMenu(int value){
  previous_selection = current_selection;
  current_selection += value;
  if(current_selection > 4){
    current_selection = 1;
  }
  if(current_selection < 1){
    current_selection = 4;
  }
  drawMenuCursor(previous_selection, current_selection);
}

void gotoSubMenu(int selection){
  switch(selection){
    case 1:
      updateBacklight();
      break;
    case 2:
      updateContrast();
      break;
    case 3:
      updateLedState();
      break;
    case 4:
      reset();
      break;
  }
}

void updateBacklight(){  //Backlight menu loop
  boolean quit = false;
  drawBacklightMenu();
  while(!quit){
    switch(readKey()){
      case 67:  //right
        changeBacklightColour(1);
        break;
      case 68:  //left
        changeBacklightColour(-1);
        break;
      case 69:  //centre
        quit = true;
        break;
    }
    delay(100);
  }
}

void drawBacklightMenu(){
  char* menu_strings[] = { "~Backlight Settings", "", "'<' and '>' change", "'o' exits to menu" };
  clearScreen();
  for(int i = 0; i < 4; i++){
    writeMessage(menu_strings[i], strlen(menu_strings[i]), 1, i+1);
  }
}

void changeBacklightColour(int selection){
  selected_colour += selection;
  if(selected_colour > 7){
    selected_colour = 0;
  }
  if(selected_colour < 0){
    selected_colour = 7;
  }
  setBacklightColour(backlight_colours[selected_colour][0], backlight_colours[selected_colour][1], backlight_colours[selected_colour][2]);
}

void updateContrast(){  //Contrast menu loop
  boolean quit = false;
  drawContrastMenu();
  while(!quit){
    switch(readKey()){
      case 67:  //right
        changeContrast(32);
        break;
      case 68:  //left
        changeContrast(-32);
        break;
      case 69:  //centre
        quit = true;
        break;
    }
    delay(100);
  }
}

void changeContrast(int change){
  contrast += change;
  if(contrast > 255){
    contrast = 0;
  }
  if(contrast < 0){
    contrast = 255;
  }
  setContrast(contrast);
}

void drawContrastMenu(){
  char* menu_strings[] = { "~Contrast Settings", "", "'<' and '>' change", "'o' exits to menu" };
  clearScreen();
  for(int i = 0; i < 4; i++){
    writeMessage(menu_strings[i], strlen(menu_strings[i]), 1, i+1);
  }
}

void updateLedState(){  //LED menu loop
  boolean quit = false;
  drawLedMenu();
  while(!quit){
    switch(readKey()){
      case 66:  //up
        changeLedState(-1,0);
        break;
      case 72:  //down
        changeLedState(1,0);
        break;
      case 67:  //right
        changeLedState(0,1);
        break;
      case 68:  //left
        changeLedState(0,-1);
        break;
      case 69:  //centre
        quit = true;
        break;
    }
    delay(100);
  }
}

void drawLedMenu(){
  char* menu_strings[] = { "~LED Settings", "'^' and 'v' led", "'<' and '>' colour", "'o' exits to menu" };
  clearScreen();
  for(int i = 0; i < 4; i++){
    writeMessage(menu_strings[i], strlen(menu_strings[i]), 1, i+1);
  }
}

void changeLedState(int led, int state){
  selected_led += led;
  led_state += state;
  if(selected_led > 3){
    selected_led = 1;
  }
  if(selected_led < 1){
    selected_led = 3;
  }
  if(led_state > 3){
    led_state = 0;
  }
  if(led_state < 0){
    led_state = 3;
  }
  setLedState(selected_led, led_state);
}

void reset(){  //Reset display to defaults
  setBacklightColour(255,255,255);
  setContrast(128);
  setLedState(1,0);
  setLedState(2,0);
  setLedState(3,0);
}

byte readKey(){
  pollKeyBuffer();
  return(i2cRead());
}

void writeMessage(char* data, byte length, byte col, byte row){
  setCursorPosition(col, row);
  i2cWrite(data, length);
}

void setLedState(byte led, byte state)
{
  switch(state){  //Set states for bicolour LEDs 1, 2, & 3
  case 0:
    setLedOff(led*2-1);
    setLedOff(led*2);
    break;
  case 1:
    setLedOff(led*2-1);
    setLedOn(led*2);
    break;
  case 2:
    setLedOn(led*2-1);
    setLedOff(led*2);
    break;
  case 3:
    setLedOn(led*2-1);
    setLedOn(led*2);
    break;
  }
}

void clearScreen(){
  char command[] = { 254, 88 };
  i2cWrite(command, sizeof(command));
}

void setCursorPosition(byte col, byte row){
  char command[] = { 254, 71, col, row };
  i2cWrite(command, sizeof(command));
}

void setBacklightColour(byte red, byte blue, byte green){
  char command[] = { 254, 130, red, green, blue };
  i2cWrite(command, sizeof(command));
}

void setContrast(byte contrast){
  char command[] = { 254, 80, contrast };
  i2cWrite(command, sizeof(command));
}

void setLedOn(byte led){
  char command[] = { 254, 87, led };
  i2cWrite(command, sizeof(command));
}

void setLedOff(byte led){
  char command[] = { 254, 86, led };
  i2cWrite(command, sizeof(command));
}

void setReportingMode(byte mode){
  char command[] = { 254, 160, mode };
  i2cWrite(command, sizeof(command));
}

void autoTxKeysOff(){
  char command[] = { 254, 79 };
  i2cWrite(command, sizeof(command));
}

void pollKeyBuffer(){
  char command[] = { 254, 38 };
  i2cWrite(command, sizeof(command));
}

void i2cWrite(char* data, byte length){  //End transmission sends bytes queued by write function
  Wire.beginTransmission(I2C_Address);
  for(int i = 0; i < length; i++){
    Wire.write(data[i]);
  }
  Wire.endTransmission();
}

byte i2cRead(){  //Wait for one byte to be read over i2c
  Wire.requestFrom(I2C_Address, 1);
  while(Wire.available()<1){
  }
  return(Wire.read());
}


