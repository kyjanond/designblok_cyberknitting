// Adjust NUMPIXELS to change the number of pixels in the system. 
// Adjust FADE_FREQ to change the fade duration (lower is slower).
// D7 pin is normally opened. D8 pin is normally closed. 
// library used: https://www.arduino.cc/reference/en/libraries/adafruit-mpr121/ , https://www.arduino.cc/reference/en/libraries/adafruit-neopixel/

#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <Wire.h>
#include "Adafruit_MPR121.h"

#ifdef __AVR__
  #include <avr/power.h>
#endif

#ifndef _BV
#define _BV(bit) (1 << (bit)) 
#endif

//NEOPIXELS
#define NEOPIXEL_PIN  6
#define NUMPIXELS     52 //ADJUST THIS: nr of pixels in the system
#define DELAYVAL      40
#define FADE_FREQ     0.5f //ADJUST THIS: higher number is faster fade
const uint8_t LVLTOUCH = 255;
const uint8_t LVLNOTOUCH = 0;
const uint8_t STEP = (LVLTOUCH-LVLNOTOUCH)*(DELAYVAL/(1000.0f/FADE_FREQ));
Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

//MPR121
#define SIG_NO 7
#define SIG_NC 8
Adafruit_MPR121 cap = Adafruit_MPR121();

// Keeps track of the last pins touched
// so we know when buttons are 'released'
uint16_t lasttouched = 0;
uint16_t currtouched = 0;

unsigned long pixel_zero_time = 1000.0/FADE_FREQ;
unsigned long last_time = 0;
unsigned long current_time = 0;
long br_raw = 0;

bool is_touching = false;

void setup() {
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif

  //serial
  //Serial.begin(9600);
  while (!Serial) { // needed to keep leonardo/micro from starting too fast!
    delay(10);
  }

  //NEOPIXELS
  pixels.begin();
  pixels.fill(pixels.Color(LVLNOTOUCH,0,0),0,NUMPIXELS);
  pixels.show();
  
  //MPR121
  Serial.println("Adafruit MPR121 Capacitive Touch sensor test"); 
  
  // Default address is 0x5A, if tied to 3.3V its 0x5B
  // If tied to SDA its 0x5C and if SCL then 0x5D
  if (!cap.begin(0x5A,&Wire,14U,6U)) {
    Serial.println("MPR121 not found, check wiring?");
    while (1);
  }
  Serial.println("MPR121 found!");

  //debug
  pinMode(LED_BUILTIN, OUTPUT); //touch led
  pinMode(SIG_NC, OUTPUT);
  pinMode(SIG_NO, OUTPUT);
  digitalWrite(SIG_NC, HIGH);
  digitalWrite(SIG_NO, LOW);
}

void loop() {
  current_time = millis();
  read_sensor();
  run_pixels();
}

void read_sensor(){
  // Get the currently touched pads
  currtouched = cap.touched();
  
  for (uint8_t i=0; i<12; i++) {
    // it if *is* touched and *wasnt* touched before, alert!
    if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {
      digitalWrite(LED_BUILTIN,HIGH);
      digitalWrite(SIG_NC, LOW);
      digitalWrite(SIG_NO, HIGH);
      is_touching = true;
      pixel_zero_time = current_time;
    }
    // if it *was* touched and now *isnt*, alert!
    if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {
      digitalWrite(LED_BUILTIN,LOW);
      digitalWrite(SIG_NC, HIGH);
      digitalWrite(SIG_NO, LOW);
      is_touching = false;
      pixel_zero_time = current_time;
    }
  }

  // reset our state
  lasttouched = currtouched;
  return; // comment out this line for detailed data from the sensor!
  // debugging info, what
  Serial.print("\t\t\t\t\t\t\t\t\t\t\t\t\t 0x"); 
  Serial.println(cap.touched(), HEX);
  Serial.print("Filt: ");
  for (uint8_t i=0; i<12; i++) {
    Serial.print(cap.filteredData(i)); Serial.print("\t");
  }
  Serial.println();
  Serial.print("Base: ");
  for (uint8_t i=0; i<12; i++) {
    Serial.print(cap.baselineData(i)); Serial.print("\t");
  }
  Serial.println();
  
  // put a delay so it isn't overwhelming
  delay(100);
}

void run_pixels(){
  if (current_time-last_time>DELAYVAL)
  {
    last_time = current_time;
    if (is_touching){
      br_raw += STEP;
    }
    else{
      br_raw -= STEP;
    }
    
    if (br_raw>LVLTOUCH){
      br_raw = LVLTOUCH;
    }
    else if(br_raw<LVLNOTOUCH){
      br_raw = LVLNOTOUCH;
    }
    uint8_t act_br = br_raw;
    //pixels.clear();
    pixels.fill(pixels.Color(act_br,0,(255-act_br)*0.2),0,NUMPIXELS);
    pixels.show();
  }
}