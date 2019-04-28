




//Pins
#define THR_IN_PIN 0 //A0
#define THR_OUT_PIN 5 //D5
#define BATT_PIN 1 //A1
#define VOLT_ARDUINO 5 //arduino 5V or 3.3V


//Throttle constants. Input constants should be less range than the sensor values, so they get coerced to the same range always, with some deadband. 
//The output constants should be larger range than the cycle expects to achieve full range. 
//Most important, do not raise thrOutMin too high, or the motor will always run. 
#define thrInMin  46
#define thrInMax  35
#define thrOutMin 20
#define thrOutMax 70

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
  Serial.begin(9600);

}

void loop() 
{
  
  float thrHallPer= analogRead(THR_IN_PIN) * (100.0 / 1023.0);
  float battVoltage = calcVoltage(analogRead(BATT_PIN)) ;
  

  int scaledThrottlePer = adjustThr(thrHallPer);
  
  analogWrite(THR_OUT_PIN,(prevThrOut*2.55));
  prevThrOut = 0.75*prevThrOut + 0.25*scaledThrottlePer;
  delay(50);

}
