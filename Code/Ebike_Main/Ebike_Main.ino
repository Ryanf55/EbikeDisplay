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




//Voltage input: 12V via stepdown from power wire into RAW

//Pins ****************************************************************************************
#define THR_IN_PIN 0 //A0
#define THR_OUT_PIN 5 //D5
#define BATT_PIN 1 //A1
#define VOLT_ARDUINO 3.3 //arduino 5V or 3.3V

//Throttle constants. Input constants should be less range than the sensor values, so they get coerced to the same range always, with some deadband. 
//The output constants should be larger range than the cycle expects to achieve full range. 
//Most important, do not raise thrOutMin too high, or the motor will always run. 
#define thrInMin  40 //min throttle, no speed
#define thrInMax  80 //max throttle, high speed
#define thrOutMin 20 //min speed motor, motor turns on @ ~25 so leave it ~20
#define thrOutMax 100 //max speed motor


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





void draw(byte pageNum,float d1) 
{
 
  u8g2.firstPage();
  do 
  {

    
    if (pageNum == 1) 
    { //draw voltage and current
      u8g2.setFont(u8g2_font_profont12_tf);
      u8g2.print(d1);
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
  float outVal =  slope*(rawVal - thrInMax) + thrOutMax;
  if (outVal < thrOutMin)  //no use sending out signals lower than minThrottle
    return thrOutMin;
  else if (outVal > thrOutMax) //overflow of expected max speed command will stop the motor. 
    return thrOutMax;
  else
  return outVal;
  return -1;
}

float calcVoltage(float v)
{
  //multiplier = 21.2 (Calibated)
  return v*21.20*VOLT_ARDUINO/1023.0;
}




void setup() 
{
  
  u8g2.begin(); //Start Display
}

void loop() 
{
  /*
  // put your main code here, to run repeatedly:
  String readStr;

  //Read incoming serial data. Expects commands to be formatted as "!XXXXXXX;\n ", where XXX is any alphanumeric char
  if (Serial.available() > 0)
  {
    readStr = Serial.readStringUntil('\n');
    parseSerialReadString(readStr);
  } 
  else 
  {
    readStr = "";
  }
  */

  //Calculate Sensor Values
  float thrHallPer= analogRead(THR_IN_PIN) * (100.0 / 1023.0);
  float battVoltage = calcVoltage(analogRead(BATT_PIN)) ;
  

  int scaledThrottlePer = adjustThr(thrHallPer);
  
  analogWrite(THR_OUT_PIN,(prevThrOut*2.55));
  prevThrOut = 0.75*prevThrOut + 0.25*scaledThrottlePer;

  //Update Display
  //dispPage1(battVoltage,thrHallPer,prevThrOut);
  draw(1,12.23);
  
  delay(50);  
}


/*


//Parses a serial input string to change settings or request settings. 
void parseSerialReadString(String myS)
{
  if (myS.length() == 0) 
  {
  //do nothing
  }
  else if(myS[myS.length()-1] != ';' | myS[0] != '!') 
  {
      Serial.println("Error Reading Command. Start commands with ! and end commands with ;");

      Serial.print("Your Command:");
      Serial.print(myS);
      Serial.println();
   
  } 
  else 
  { //Parse
    //Remove ! and ;
    myS.toLowerCase();
    myS = myS.substring(1,myS.length()-1);
   

    //Scan if first word is help, get, set, or dump
    if (myS.startsWith("get")) 
    { // *******                   Getters *************
      if (myS.endsWith(" brightness")) 
      {
        Serial.println("Brightness = 9000");
      }
      else if(myS.endsWith("voltage_min"))
      {
        
      }
    }
    else if (myS.startsWith("set")) //**********            Setters ********
    {
      Serial.println("Setting a param of FIXMESETPARAM");
    }
    else if (myS == "help")
    {
      //print help text
    }
    else if(myS == "dump")
    {
      dumpSettings();
    }
    else
    {
      Serial.println("Unrecognized Command");
    }
  }
}

//dumpSettings returns the configuration data out the serial port. 
void dumpSettings()
{
  Serial.println("Dump:");
  Serial.println("Fixme dumpSettings");
}

*/
