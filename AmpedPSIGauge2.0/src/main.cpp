#include <Arduino.h>


// Simple I2C test for ebay 128x64 oled.

#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#include <EEPROM.h>
// Below is setup for ATTiny1626
#define PRES_ADC A7
#define OUT_DASH 14
// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3C

// Define proper RST_PIN if required.
#define RST_PIN -1

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
      digitalWrite(OUT_DASH, HIGH);
  }
  else
  {
      digitalWrite(OUT_DASH, LOW);
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
 
  if(average_pressure > 0.0F){
    oled.print(average_pressure);
  }
  else{
    oled.print(0.0F);   
  }
  oled.set1X();
  oled.setFont(Adafruit5x7);
}



//------------------------------------------------------------------------------
void setup() {
  unsigned long curMillis = 0;

  pinMode(OUT_DASH, OUTPUT);
  pinMode(PRES_ADC, INPUT);
  EEPROM.get<unsigned short>(0, dacMin);
  
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
    dacAdjusted = true;
    EEPROM.put<unsigned short>(0, dacMin);
    EEPROM.put<bool>(sizeof(unsigned short), dacAdjusted);
    
  }
  dacMax -= dacMin - 101; 
  oled.clear();
  oled.println(F("Oil:"));
  oled.set1X();
  oled.setFont(Adafruit5x7);
  oled.println("DAC RAW:");
  oled.println(dacMin);
  digitalWrite(OUT_DASH, LOW);
  
  delay(2000);  
}

//------------------------------------------------------------------------------
void loop() {
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
