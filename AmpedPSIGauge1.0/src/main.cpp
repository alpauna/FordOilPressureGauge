#include <Arduino.h>


// Simple I2C test for ebay 128x64 oled.

#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#include <EEPROM.h>
#define PRES_ADC A2
SSD1306AsciiWire oled;
unsigned short dacMin = 101;
unsigned short dacMax = 923;
unsigned long lastMSForDacRead = 0;
unsigned long lastMSForUpdateLCD = 0;
bool dacAdjusted = false;

long getScaledValue(long value)
{
  return map(value, dacMin, dacMax, 0, 1000);
}
/*
float mapf(float x, float in_min, float in_max, float out_min, float out_max) 
{   
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min; 
} 

float getScaledValue(float value)
{
  return map(value, dacMin, 923.0, 0.0F, 100.0F);
}
*/

float average_pressure = 0.0F;

void getPressureADC() {
  unsigned short rawDac = analogRead(PRES_ADC); 
  float val = (float)getScaledValue(rawDac)/10.0; 
  if(average_pressure > 0.0F){
    average_pressure = (average_pressure + val) / 2; 
  }
  else{
    average_pressure = val; 
  }
  if(val < 10.0F)
  {
      digitalWrite(1, HIGH);
  }
  else
  {
      digitalWrite(1, LOW);
  }
  if(rawDac < dacMin  && dacAdjusted == false)
  {
    dacMin--; 
  }
  if(average_pressure > 1.0   && dacAdjusted == false)
  {
    dacMin++; 
  }
  oled.clearField(0, 7, 5);
  oled.print(rawDac); 
}

void updatePressure()
{
  oled.setFont(fixed_bold10x15); 
  oled.set2X();
  oled.clearField(0, 2, 7);  
  //sprintf(buffer, "%3.1g", (double)mapf( analogRead(2), 50.0, 1023.0, 0.0, 100.0));
  if(average_pressure > 0.0F){
    oled.print(average_pressure);
  }
  else{
    oled.print(0.0F);   
  }
  oled.set1X();
  oled.setFont(Adafruit5x7);
}

//Ticker tickerGP(getPressureADC, 1000, 0, MILLIS);
///Ticker tickerUD(updatePressure, 3000, 0, MILLIS);

// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3C

// Define proper RST_PIN if required.
#define RST_PIN -1


//uint8_t fields[2]; // Columns for ADC values.
//char buffer[20];
//------------------------------------------------------------------------------
void setup() {
  unsigned long curMillis = 0;
  //tickerGP.start();
  //tickerUD.start();
  pinMode(1, OUTPUT);
  pinMode(4, INPUT);
  EEPROM.get<unsigned short>(0, dacMin);
  //EEPROM.get<bool>(sizeof(unsigned short), dacAdjusted);
  
  Wire.begin();
  Wire.setClock(400000L);
  

#if RST_PIN >= 0
  oled.begin(&Adafruit128x64, I2C_ADDRESS, RST_PIN);
#else // RST_PIN >= 0
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
#endif // RST_PIN >= 0

  oled.setFont(fixed_bold10x15);
  oled.clear();
  curMillis = millis();
  if(dacMin > 300){
    dacAdjusted = false;   
    dacMin = 101;
  }else
  {
    dacAdjusted = true; 
     
  }
  if(!dacAdjusted){
    while(curMillis + millis() < curMillis + 30000){
      getPressureADC(); 
      delay(500);
    }
  
    //oled.clear();
    dacAdjusted = true;
    EEPROM.put<unsigned short>(0, dacMin);
    EEPROM.put<bool>(sizeof(unsigned short), dacAdjusted);
    
  }
  dacMax -= dacMin - 101; 
  oled.clear();
  // Setup form.  Could use F() macro to save RAM on AVR.
  oled.println(F("Oil:"));
  oled.set2X();
  //sprintf(buffer, "%3.1g", (double)mapf( (float)analogRead(2), 50.0, 1023.0, 0.0F, 100.0F));
  //oled.println(getScaledValue((float)analogRead(2)));
  oled.set1X();
  oled.setFont(Adafruit5x7);
  oled.println("");
  oled.println(dacMin);
  digitalWrite(1, LOW);
  
  // Calculate columns for ADC values.  No RAM is used by strings.
  // Compiler replaces strlen() calc with 6 and 17.  
  //fields[0] = oled.fieldWidth(strlen("ADC0: "));
  //col[1] = oled.fieldWidth(strlen("ADC0: 9999 ADC1: "));
  delay(2000);  
}

//------------------------------------------------------------------------------
void loop() {
  
  //tickerGP.update();
  //tickerUD.update();
  unsigned long ticks = millis();
  if(ticks - lastMSForDacRead > 1000)
  {
    getPressureADC();
    lastMSForDacRead = ticks;   
  }
  if(ticks - lastMSForUpdateLCD > 3000){
    updatePressure();
    lastMSForUpdateLCD = ticks;
  }
  
}
