#include <Arduino.h>
#include <EEPROM.h>
#include <TinyWireM.h> 
#define TINY4KOLED_QUICK_BEGIN
#include <Tiny4kOLED.h>
#include <Button.h>
//#include "RunningAverage.h"

//RunningAverage myRA(5);

#define PRES_ADC A2
#define BUTTON_PIN PB3
#define DAC_MIN_REF 90
#define DAC_MAX_REF 860
#define MIN_DAC_INIT_CHECK_MS 30000
#define OUT_DASH PB1
#define DAC_CONV 5.0/1023.0

unsigned short dacMin = DAC_MIN_REF;
unsigned short dacMax = DAC_MAX_REF;
unsigned short rawDac = 0;
bool last_button_state = false;
bool dacAdjusted = false;
bool mainMenuEnabled = false;
bool longPress = false;
unsigned long uTickTimeGetPress, uTickTimeUpdateLCD, uTickMainMenu, uTickCheck, uTickButtons,uTickButtonPressed;

float average_pressure = 0.0F;


void updatePressureInit(unsigned short dac);
void getPressureADC(void);
void PrintText(const __FlashStringHelper * text);

Button button(BUTTON_PIN);

float getScaledValue(unsigned short value)
{
  
  return (float)map(value, dacMin, dacMax, 0, 1000) / 10.0;
}
long getScaledScreen(unsigned short value)
{
  
  return map(value, 0, 1000, 0, 220) / 10;
}

void SetDacMin()
{
  EEPROM.get<unsigned short>(0, dacMin);
  EEPROM.get<unsigned short>(sizeof(unsigned short), dacMax);
   if(dacMin > DAC_MIN_REF * 2){
    dacMax = DAC_MAX_REF;
    dacAdjusted = false;   
    dacMin = DAC_MIN_REF;
  }else
  {
    dacAdjusted = true; 
     
  }
  unsigned long curMillis = millis();
  unsigned long curMillis2 = curMillis ;
  if(!dacAdjusted){
    
    while(millis() - curMillis <  MIN_DAC_INIT_CHECK_MS){
      delay(200);
      unsigned short dac = analogRead(PRES_ADC);
      if(dac < dacMin && dac > 0)
      {
        dacMin--;
      }
      if(dac > dacMin && dac < 200){
        dacMin++;
      }
      
      if(millis() - curMillis2 >= 1000){
        updatePressureInit(dacMin);
        curMillis2 = millis();
      } 
    }
    dacMax -= DAC_MIN_REF - dacMin;
    dacAdjusted = true;
    EEPROM.put<unsigned short>(0, dacMin);
    EEPROM.put<unsigned short>(sizeof(unsigned short), dacMax);
  }
}

void getPressureADC() {
  rawDac = analogRead(PRES_ADC); 
  //myRA.addValue(rawDac);
  float val = 0;
  //average_pressure = getScaledValue(myRA.getAverage()); 
  if(rawDac >= dacMin){
    val = getScaledValue(rawDac); 
  }
  if(average_pressure > 0.0F){
    average_pressure = (average_pressure + val) / 2; 
  }
  else{
    average_pressure = val; 
  }
  if(average_pressure < 10.0F)
  {
      digitalWrite(1, HIGH);
  }
  else
  {
      digitalWrite(1, LOW);
  }
}

void updatePressureInit(unsigned short dac)
{
  oled.clear();
  oled.setFont(FONT6X8);
  oled.println(F("Zeroing..."));
  oled.print(F("Dac:"));
  //float val = DAC_CONV * (float)dac;
  oled.println(dac);
  oled.switchFrame();
}

void updatePressure()
{
  oled.clear();
  oled.setFont(FONT6X8);
  oled.println(F("Oil Pressure: "));
  oled.setFont(FONT8X16);
  oled.setCursor(40,1);
  //oled.setFontX2(FONT6X8);
  oled.println(average_pressure);
  oled.setFont(FONT6X8);
  //oled.println(getScaledScreen(average_pressure));
  oled.setCursor(0,3);
  for(int i = 0; i < getScaledScreen(average_pressure*10); i++ ){
    oled.print(F("="));
  }
  oled.switchFrame();
}
void EditModeTick();

void showMenu()
{
  PrintText(F("Reset? Press and Hold."));
}

void lpStart(){
  if(!mainMenuEnabled){
    mainMenuEnabled = true;
    uTickMainMenu = millis();
      showMenu();
  }else{
    EEPROM.put<unsigned short>(0, 0xFFFF);
    EEPROM.put<unsigned short>(sizeof(unsigned short), 0xFFFF);
    dacMin = DAC_MIN_REF;
    dacMax = DAC_MAX_REF;
    SetDacMin(); 
  }  
}
void EditModeTick()
{
  mainMenuEnabled = false;
}

void PrintText(const __FlashStringHelper * text){
  oled.clear();
  oled.setFont(FONT8X16);
  oled.println(text);
  oled.switchFrame();
}

//------------------------------------------------------------------------------
void setup() {
  
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(OUT_DASH, OUTPUT);
  pinMode(PRES_ADC, INPUT);
  digitalWrite(OUT_DASH, HIGH);

  oled.begin(0,0);
  oled.enableChargePump();
  oled.setRotation(1);
  oled.switchRenderFrame();
  oled.enableZoomIn();
  PrintText(F("AMPED!!!"));
  oled.on();
  delay(2000);
  SetDacMin();
  digitalWrite(OUT_DASH, LOW);
  getPressureADC();
  updatePressure();

  button.begin();
  
}

//------------------------------------------------------------------------------
void loop() {
  uTickCheck = millis();
  if(uTickMainMenu > uTickCheck){
    // This happens when millis runs over and starts over so start all of the ticks over.
    unsigned long change = uTickMainMenu - uTickCheck;
    uTickMainMenu = change;
    uTickTimeGetPress = change;
    uTickTimeUpdateLCD = change;
  }

  if(millis() - uTickTimeGetPress >= 200 )
  {
    getPressureADC();
    uTickTimeGetPress = millis();
  }
  if(millis() - uTickTimeUpdateLCD >= 500 && !mainMenuEnabled)
  {
    updatePressure();
    uTickTimeUpdateLCD = millis();
  }
  if(mainMenuEnabled && millis() - uTickMainMenu >= 30000){
    EditModeTick();
    uTickMainMenu = millis();
  }
  if(button.pressed() && !last_button_state){
    last_button_state = true;
    uTickButtonPressed = millis();
  }
  if(last_button_state &&  millis() - uTickButtonPressed >= 5000){
    lpStart(); 
    last_button_state = false;  
    uTickButtonPressed = 0; 
  }
  if(button.released()){
    last_button_state = false;  
    uTickButtonPressed = 0;
  }
}
