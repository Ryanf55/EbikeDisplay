//Electric Bike Controller for Cyclone
//Ryan Friedman
//1-10-2019

/*
 * Todo:
 * Change display font size for voltage
 * Speed sensor
 * Write engine for data display (Struct of name -  value -  unitText). Only print units and lines when menu is changed. 
 *  Struct is made
 *  Need to write getters and setters
 * Write engine for datalines
 * Write engine for 1 button scrolling
 * Link up parameters with data display struct
 * 
 * */

#define LARGE_FONT u8g2_font_profont17_tf
#define MEDIUM_FONT u8g2_font_profont15_tf
#define SMALL_FONT u8g2_font_profont12_tf


//Voltage input: 12V via stepdown from power wire into RAW

//Pins ****************************************************************************************
#define THR_IN_PIN 0 //A0,
#define THR_OUT_PIN 6 //PWM pin, labeled 6
#define BATT_PIN 1 //A1


//Throttle constants. Input constants should be less range than the sensor values, so they get coerced to the same range always, with some deadband. 
//The output constants should be larger range than the cycle expects to achieve full range. 
//Most important, do not raise thrOutMin too high, or the motor will always run. 
#define thrInMin  51 //min throttle, no speed
#define thrInMax  75 //max throttle, high speed
#define thrOutMin 20 //min speed motor, motor turns on @ ~25 so leave it ~20
#define thrOutMax 100 //max speed motor
#define ALPHA 0.10 //alpha for expo filter, between 0 and 1. If alpha is 1, weights latest sample only, whereas low alpha smooths a lot

//Voltages *********************
#define VOLT_ARDUINO 3.3 //arduino 5V or 3.3V
#define BAT_CELLS 14
#define VPC_MAX 4.2 // volts per cell, max for li-ion
#define VPC_MIN 3.6 // volts per cell, min for li-ion
#define VOLT_MAX (VPC_MAX * BAT_CELLS)
#define VOLT_MIN (VPC_MIN * BAT_CELLS)


//Display *********************************************************************************
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

// U8g2 Contructor List (Picture Loop Page Buffer)
// The complete list is available here: https://github.com/olikraus/u8g2/wiki/u8g2setupcpp
// Please update the pin numbers according to your setup. Use U8X8_PIN_NONE if the reset pin is not connected

U8G2_PCD8544_84X48_1_4W_SW_SPI u8g2(U8G2_R2, /* clock (SCLK)=*/ 13, /* data (DN(MOSI))=*/ 11, /* cs (SCE) =*/ 10, /* dc (D/C)=*/ 9, /* reset (RST)=*/ 8);  // Nokia 5110 Display
//U8G2_PCD8544_84X48_1_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);           // Nokia 5110 Display





void draw(
  byte pageNum,
  float main_voltage,
  float thrHallPer,
  float aout,
  float thrFiltered,
  float battPercent
  ) 
{
 
  u8g2.firstPage();
  do 
  {

    
    if (pageNum == 1) 
    { //draw voltage and current
//      u8g2.setFont(LARGE_FONT);
//      u8g2.setCursor(0,17);
//      u8g2.print(main_voltage);
//      u8g2.setFont(SMALL_FONT);
//      u8g2.print("vb/");
//      u8g2.print(VOLT_MAX);
//
//      u8g2.setFont(MEDIUM_FONT);
//      u8g2.setCursor(0,25);
//      u8g2.print(thrHallPer);
//      u8g2.setFont(SMALL_FONT);
//      u8g2.print("thrHall");
//
//      u8g2.setFont(SMALL_FONT);
//      u8g2.setCursor(0,37);
//      u8g2.print(aout);
//      u8g2.setFont(SMALL_FONT);
//      u8g2.print("vo");
//
//      u8g2.setFont(SMALL_FONT);
//      u8g2.setCursor(0,49);
//      u8g2.print(thrFiltered);
//      u8g2.setFont(SMALL_FONT);
//      u8g2.print("vfilter");

        u8g2.setFont(LARGE_FONT);
        u8g2.setCursor(0,17);
        u8g2.print(battPercent);
        u8g2.setFont(SMALL_FONT);
        u8g2.print("%");

        u8g2.setFont(MEDIUM_FONT);
        u8g2.setCursor(0,30);
        u8g2.print(main_voltage);
        u8g2.setFont(SMALL_FONT);
        u8g2.print("v");
    }
    else 
    {
      u8g2.setFont(u8g2_font_profont12_tf);
      u8g2.drawStr(0,8,"Page Num");
      u8g2.drawStr(0,17,"Error");
    }
    
    
   
   
  } while ( u8g2.nextPage() );
  
}



float prevThrOut = 0;

int adjustThr(float rawVal) 
{
  //Map input to output (negative slope)
  float slope = (thrOutMin - thrOutMax) / (thrInMin - thrInMax);
  float outVal =  slope*(rawVal - thrInMin) + thrOutMin;
  if (outVal < thrOutMin)  //no use sending out signals lower than minThrottle
    return thrOutMin;
  else if (outVal > thrOutMax) //overflow of expected max speed command will stop the motor. 
    return thrOutMax;
  else
  return outVal;
  return -1;
}


int expoThr(float new_sample, float ma_old){
  if (new_sample < ma_old)
  {
    return new_sample;
  } else 
  {
    return ALPHA * new_sample + (1-ALPHA) * ma_old;
  }
}
  

float calcVoltage(float v)
{
  //multiplier = 21.2 (Calibated)
  return v*21.20*VOLT_ARDUINO/1023.0;
}

float calcPercentage(float batVolt)
{
  return 100 / (VOLT_MAX - VOLT_MIN) * (batVolt - VOLT_MIN);
}




void setup() 
{
  
  u8g2.begin(); //Start Display
  u8g2.setContrast(135);
}

float thr_old = thrOutMin;
void loop() 
{

  //Calculate Sensor Values
  float thrHallPer= analogRead(THR_IN_PIN) * (100.0 / 1023.0);
  // 43 low throttle, 78 high throttle
  float battVoltage = calcVoltage(analogRead(BATT_PIN)) ;
  int scaledThrottlePer = adjustThr(thrHallPer);
  thr_old = expoThr(scaledThrottlePer, thr_old);
  
 
  analogWrite(THR_OUT_PIN,((int)thr_old*2.55));
//  prevThrOut = 0.75*prevThrOut + 0.25*scaledThrottlePer;
//
//  //Update Display
//  //dispPage1(battVoltage,thrHallPer,prevThrOut);

//  
  delay(50);  
  draw(1, battVoltage, (int)thrHallPer, scaledThrottlePer, (int)thr_old, calcPercentage(battVoltage));
}
