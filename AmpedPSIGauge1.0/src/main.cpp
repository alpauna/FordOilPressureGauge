#include <Arduino.h>


// Simple I2C test for ebay 128x64 oled.

#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

#include <Ticker.h>
#define PRES_ADC A2
SSD1306AsciiWire oled;

float mapf(float x, float in_min, float in_max, float out_min, float out_max) 
{   
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min; 
} 

float getScaledValue(float value)
{
  return mapf(value, 101.0, 923.0, 0.0F, 100.0F);
}

float average_pressure = 0.0F;

void getPressureADC() {
  float rawDac = (float)analogRead(PRES_ADC); 
  float val = getScaledValue(rawDac); 
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
  oled.clearField(0, 7, 5);
  oled.print((rawDac/1024.0F) * 5.0F ); 
}

void updatePressure()
{
  oled.setFont(fixed_bold10x15); 
  oled.set2X();
  oled.clearField(0, 2, 7);  
  //sprintf(buffer, "%3.1g", (double)mapf( analogRead(2), 50.0, 1023.0, 0.0, 100.0));
  oled.print(average_pressure);
  oled.set1X();
  oled.setFont(Adafruit5x7);
}

Ticker tickerGP(getPressureADC, 1000, 0, MILLIS);
Ticker tickerUD(updatePressure, 3000, 0, MILLIS);

// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3C

// Define proper RST_PIN if required.
#define RST_PIN -1


float Vcc;


//uint8_t fields[2]; // Columns for ADC values.
uint8_t rows;   // Rows per line.
//char buffer[20];
//------------------------------------------------------------------------------
void setup() {
  tickerGP.start();
  tickerUD.start();
  pinMode(1, OUTPUT);
  pinMode(4, INPUT);
  Wire.begin();
  Wire.setClock(400000L);
  //Vcc = readVcc() / 1000.0F;

#if RST_PIN >= 0
  oled.begin(&Adafruit128x64, I2C_ADDRESS, RST_PIN);
#else // RST_PIN >= 0
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
#endif // RST_PIN >= 0

  oled.setFont(fixed_bold10x15);
  oled.clear();

  // Setup form.  Could use F() macro to save RAM on AVR.
  oled.println("Oil Press:");
  oled.set2X();
  //sprintf(buffer, "%3.1g", (double)mapf( (float)analogRead(2), 50.0, 1023.0, 0.0F, 100.0F));
  oled.println(getScaledValue((float)analogRead(2)));
  oled.set1X();
  oled.setFont(Adafruit5x7);
  oled.println("DAC Volts:");
  oled.println(Vcc);
  digitalWrite(1, LOW);
  
  // Calculate columns for ADC values.  No RAM is used by strings.
  // Compiler replaces strlen() calc with 6 and 17.  
  //fields[0] = oled.fieldWidth(strlen("ADC0: "));
  //col[1] = oled.fieldWidth(strlen("ADC0: 9999 ADC1: "));
  rows = oled.fontRows();
  delay(2000);  
}

//------------------------------------------------------------------------------
void loop() {
  
  tickerGP.update();
  tickerUD.update();
  
}
