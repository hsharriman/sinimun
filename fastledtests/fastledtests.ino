#include <FastLED.h>

//#define NUM_LEDS 20

CRGB strip1[24];
CRGB strip2[20];
CRGB strip3[8];
//CRGB strip4[10];
CRGB striparray[] = {strip1, strip2, strip3};
int num_led_array[] = {24, 20, 8};
int num_strands = 3;

int n;
int i;
bool ascent;
int testColor[3] = {150, 60, 200};

void setup() {
  FastLED.addLeds<NEOPIXEL, 7>(strip1, 24);
  FastLED.addLeds<NEOPIXEL, 8>(strip2, 20);
  FastLED.addLeds<NEOPIXEL, 9>(strip3, 8);
  //FastLED.addLeds<NEOPIXEL, 10>(strip4, NUM_LEDS);
  strip1[0].setRGB(testColor[0], testColor[1], testColor[2]);
  Serial.begin(9600);

}

void loop() {
  //  if (millis() < 7000){
  //    if (i < NUM_LEDS){
  //      breathe(strip1[i], 20, testColor[0], testColor[1], testColor[2]);
  //      breathe(strip2[i], 8, testColor[0], testColor[1], testColor[2]);
  //      breathe(strip3[i], 10, testColor[0], testColor[1], testColor[2]);
  //      //breathe(strip4[i], 20, testColor[0], testColor[1], testColor[2]);
  //
  //      i++;
  //    }
  //    if (i == NUM_LEDS){
  //      i = 0;
  //    }
  //    delay(80);
  //    fadeToBlackBy(strip1, NUM_LEDS, 60);
  //    fadeToBlackBy(strip2, NUM_LEDS, 80);
  //    fadeToBlackBy(strip3, NUM_LEDS, 100);
  //  }
  //
  //  else if (millis() > 7000 && millis() < 20000){
  //    for (i = 0; i < NUM_LEDS; i++){
  //      breatheOnOff(strip1[i], 10, 20, 20, 40, 80);
  //      breatheOnOff(strip2[i], 10, 20, 20, 40, 80);
  //      breatheOnOff(strip3[i], 10, 20, 20, 40, 80);
  //    }
  //    delay(80);
  //  }
  for (n= 0; n < num_strands; n++) {
    CRGB strip = striparray[n]; 
    Serial.println(n);
    int curNumLEDs = num_led_array[n];
    //.println(curNumLEDs);
    loopThemePerStrand(striparray[n], strip, curNumLEDs);
  }

  FastLED.show();
  Serial.println(millis());
}

void breathe(CRGB& led, int fadeOnRate, int clrR, int clrG, int clrB) {
  int onR = map(clrR, 0, 255, 0, fadeOnRate);
  int onG = map(clrG, 0, 255, 0, fadeOnRate);
  int onB = map(clrB, 0, 255, 0, fadeOnRate);

  if ((led.r + led.g + led.b) == 0) {
    ascent = true;
  }
  if (led.r >= 255 or led.g >= 255 or led.b >= 255) {
    ascent = false;
  }

  if (ascent == true) {
    led += CRGB(onR, onG, onB);
  }
}

void breatheOnOff(CRGB& led, int fadeOnRate, int fadeOffRate, int clrR, int clrG, int clrB) {
  int onR = map(clrR, 0, 255, 0, fadeOnRate);
  int onG = map(clrG, 0, 255, 0, fadeOnRate);
  int onB = map(clrB, 0, 255, 0, fadeOnRate);

  int offR = map(clrR, 0, 255, 0, fadeOffRate);
  int offG = map(clrG, 0, 255, 0, fadeOffRate);
  int offB = map(clrB, 0, 255, 0, fadeOffRate);

  if ((led.r + led.g + led.b) == 0) {
    ascent = true;
  }
  if (led.r >= 100 or led.g >= 100 or led.b >= 100) {
    ascent = false;
  }

  if (ascent == true) {
    led += CRGB(onR, onG, onB);
  }

  if (ascent == false) {
    led -= CRGB(offR, offG, offB);
  }
}

void loopThemePerStrand(CRGB strip, CRGB stripref, int num_leds) {
//  if (millis() < 7000) {
//    if (i < num_leds) {
//      CRGB led = stripref[i];
//      breathe(led, 20, testColor[0], testColor[1], testColor[2]);
//      //breathe(strip2[i], 8, testColor[0], testColor[1], testColor[2]);
//      //breathe(strip3[i], 10, testColor[0], testColor[1], testColor[2]);
//      //breathe(strip4[i], 20, testColor[0], testColor[1], testColor[2]);
//
//      i++;
//    }
//    if (i == num_leds) {
//      i = 0;
//    }
//    delay(80);
    CRGB* fullStrip = &stripref;
    fadeToBlackBy(fullStrip, num_leds, 60);
    //fadeToBlackBy(strip2, num_leds, 80);
    //fadeToBlackBy(strip3, num_leds, 100);
  //}

  /*else if (millis() > 7000 && millis() < 20000)*/
  if (millis() < 7000) {
    for (i = 0; i < num_leds; i++) {
      CRGB led = stripref[i];
      breatheOnOff(led, 10, 20, 20, 40, 80);
      //breatheOnOff(strip2[i], 10, 20, 20, 40, 80);
      //breatheOnOff(strip3[i], 10, 20, 20, 40, 80);
    }
    //delay(80);
  }
}
