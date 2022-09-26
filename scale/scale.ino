// The sensor will zero itself (tare) everytime it is switched on.
// Adjust FORCE_LIMIT to change the sensitivity.
// D7 pin is normally opened. D8 pin is normally closed. 
#include "HX711.h"

#define DOUT  2
#define CLK  3
#define SIG_NO 7
#define SIG_NC 8

#define FORCE_LIMIT -5 //change this to calibrate (roughly equals to kilograms)

HX711 scale;

float calibration_factor = -15510.0; //we use zero because we need only abstract units
float value = 0;

void setup() {
  Serial.begin(9600);
  scale.begin(DOUT, CLK);
  scale.set_scale();
  scale.tare(); //Reset the scale to 0
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(SIG_NO, OUTPUT);
  pinMode(SIG_NC, OUTPUT);

  long zero_factor = scale.read_average(); //Get a baseline reading
  //Serial.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
  //Serial.println(zero_factor);
}

void loop() {
  scale.set_scale(calibration_factor); //Adjust to this calibration factor
  value = scale.get_units(0x08);
  if (value<FORCE_LIMIT){
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(SIG_NC, LOW);
    digitalWrite(SIG_NO, HIGH);
  }
  else{
    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(SIG_NC, HIGH);
    digitalWrite(SIG_NO, LOW);
  }
  if (Serial.availableForWrite()){
    Serial.print(value, 1);
    Serial.println();
  }
}
