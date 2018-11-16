#include <FastLED.h>


#define NUM_LEDS_PART_A 20
#define NUM_LEDS_PART_B 24
#define NUM_LEDS_PART_C 20
#define NUM_LEDS_PART_D 19

#define NUM_LEDS (NUM_LEDS_PART_A + NUM_LEDS_PART_B + NUM_LEDS_PART_C + NUM_LEDS_PART_D)

#define BRIGHTNESS      64
CRGB strip1[NUM_LEDS_PART_A + NUM_LEDS_PART_B + NUM_LEDS_PART_C + NUM_LEDS_PART_D];

int n;
int i;
bool ascent;
bool flag;
bool doneBooping;

float timeSinceLastUpdate;
int currentIteration = 0;

void setup() {
  delay(3000);
  FastLED.addLeds<NEOPIXEL, 7>(strip1, 0, NUM_LEDS_PART_A);
  FastLED.addLeds<NEOPIXEL, 8>(strip1, NUM_LEDS_PART_A, NUM_LEDS_PART_B);
  FastLED.addLeds<NEOPIXEL, 9>(strip1, NUM_LEDS_PART_B, NUM_LEDS_PART_C);
  FastLED.addLeds<NEOPIXEL, 10>(strip1, NUM_LEDS_PART_C, NUM_LEDS_PART_D);
  Serial.begin(9600);
  FastLED.setBrightness(BRIGHTNESS);

}

void loop() {

  for (n = 0; n < NUM_LEDS; n++) {
    if (millis() < 4500) {
      ScaredFlash(strip1[n], 25);
    }
    else if (doneBooping == false && millis() >= 6500) {
      BoopGraph(strip1[n]);
      //breatheOnOff(strip1[n], 20, 30, 20, 40, 80);
    }

    else if (millis() > 25000 && strip1[NUM_LEDS - 1].b < 200){
      strip1[n] += CRGB(0, 0, 5); //Change color to not necessarily pure blue
    }

    else if (millis() > 30000){
      strip1[n] = CRGB::Black;
    }
  }
  if (millis() < 4500) {
    delay(150);
    Serial.println("flash");
    float timeSinceLastUpdate = millis();
  }
  else if (millis() >= 4500) {
    delay(80);
    //Serial.println("breathe");
  }
  FastLED.show();
}

void fadeIn(CRGB& led, int fadeOnRate, int clrR, int clrG, int clrB) {
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
  int onR = map(clrR, 0, 250, 0, fadeOnRate);
  int onG = map(clrG, 0, 250, 0, fadeOnRate);
  int onB = map(clrB, 0, 250, 0, fadeOnRate);

  int offR = map(clrR, 0, 250, 0, fadeOffRate);
  int offG = map(clrG, 0, 250, 0, fadeOffRate);
  int offB = map(clrB, 0, 250, 0, fadeOffRate);

  if ((led.r + led.g + led.b) == 0) {
    ascent = true;

  }
  if (led.r >= 250 or led.g >= 250 or led.b >= 250) {
    ascent = false;
    flag = true;
  }

  if (ascent == true) {
    led += CRGB(onR, onG, onB);
  }

  if (ascent == false) {
    led -= CRGB(offR, offG, offB);
  }
}

void ScaredFlash(CRGB& led, int rate) {
  if (led.r > 25) {
    led = CRGB::Black;
  }
  else if (led == CRGB(0, 0, 0)) {
    led = CRGB::Red;
  }
}

void BoopGraph(CRGB& led) {
  int colorarray[] = {250, 0, 0, 
                      200, 0, 50, 
                      150, 0, 100, 
                      100, 0, 150,
                      50,  0, 200, 
                      0,   0, 250};
  
  float currentInterval = millis() - timeSinceLastUpdate;
  if (currentIteration == 6){
    doneBooping = true; 
  }
  
  if (currentInterval >= 1000){
      breatheOnOff(led, 40, 40, colorarray[currentIteration * 3], colorarray[currentIteration * 3 + 1], colorarray[currentIteration * 3 + 2]); //change colors to go slowly from red to blue
  }
  
  if (n == (NUM_LEDS - 1)) {
    if (flag == true && (led.r + led.g + led.b) == 0){
      timeSinceLastUpdate = millis(); //Reset the delay timer
      flag = false;
      Serial.println("Resetting");
      currentIteration++;
    }
  }
}
