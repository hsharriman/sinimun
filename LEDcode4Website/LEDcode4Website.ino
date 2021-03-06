#include <FastLED.h>

//Define number of LEDs per strip
#define NUM_LEDS_PART_1 24
#define NUM_LEDS_PART_2 20
#define NUM_LEDS_PART_3 18
#define NUM_LEDS_PART_4 20
#define NUM_LEDS_PART_5 10
#define NUM_LEDS_PART_6 12
#define NUM_LEDS_PART_7 17
#define NUM_LEDS_BOT 35

//Define number of LEDs in overall strip
#define NUM_LEDS (NUM_LEDS_PART_1 + NUM_LEDS_PART_2 + NUM_LEDS_PART_3 + NUM_LEDS_PART_4 + NUM_LEDS_PART_5 + NUM_LEDS_PART_6 + NUM_LEDS_PART_7 + NUM_LEDS_BOT)
#define BRIGHTNESS      80

//Define strip object
CRGB ledstrip[NUM_LEDS];

//Used for stoic behavior
float fadingInt[76];
int remainingInt[76];

//Signal pins, receive commands from Arduino 1
const int ultraPin = 12;
const int micPin = 13;
unsigned long desiredChangeRate = 80; //update rate for LEDs
unsigned long flashRate = 120;    //update rate for LEDs for scared

bool dist = false;  //Stores last reading from Arduino 1
bool vel = false;   //Stores last reading from Arduino 1

//Used for timers
unsigned long timeSinceLastFlash = 0;
unsigned long timeSinceLastChange = 0;
int numFlashes = 8; //number of flashes for each scared animation

void setup() {
  delay(3000);  //delay 3 seconds for safety purposes

  //Add LED strips
  FastLED.addLeds<NEOPIXEL, 9>(ledstrip, 0, NUM_LEDS_PART_1);
  FastLED.addLeds<NEOPIXEL, 3>(ledstrip, NUM_LEDS_PART_1, NUM_LEDS_PART_2);
  FastLED.addLeds<NEOPIXEL, 4>(ledstrip, NUM_LEDS_PART_2, NUM_LEDS_PART_3);
  FastLED.addLeds<NEOPIXEL, 5>(ledstrip, NUM_LEDS_PART_3, NUM_LEDS_PART_4);
  FastLED.addLeds<NEOPIXEL, 6>(ledstrip, NUM_LEDS_PART_4, NUM_LEDS_PART_5);
  FastLED.addLeds<NEOPIXEL, 10>(ledstrip, NUM_LEDS_PART_5, NUM_LEDS_PART_6);
  FastLED.addLeds<NEOPIXEL, 11>(ledstrip, NUM_LEDS_PART_6, NUM_LEDS_PART_7);
  FastLED.addLeds<NEOPIXEL, 2>(ledstrip, NUM_LEDS_PART_7, NUM_LEDS_BOT);

  pinMode(ultraPin, INPUT); //Initialize input pins from Arduino 1
  pinMode(micPin, INPUT);   //Initialize input pins from Arduino 1
  Serial.begin(9600); //Initialize Serial monitor
  FastLED.setBrightness(BRIGHTNESS);  //Set global brightness
  FastLED.clear();
  
  //initialize the array for stoic behavior
  for (int i = 0; i < 76; i = i + 2) {
    fadingInt[i] = random(10, 20);
    remainingInt[i] = fadingInt[76];
  }
}

void loop() {
  //Read incoming signal
  bool ultraVal = digitalRead(ultraPin);
  bool micVal = digitalRead(micPin);
  
  //if signal is different than last one, update state, otherwise keep same
  if (ultraVal != dist) {
    dist = ultraVal;
  }
  if (micVal != vel) {
    vel = micVal;
  }

  //SCARED
  if (dist == 1 && vel == 1) {
    if (millis() - timeSinceLastFlash > flashRate) {   //Regulate frequency of flashes
      //Switch between all LEDs flashing and red "sparkling"
      for (int i = 0; i < NUM_LEDS; i++) {
        //red flashing
        if (numFlashes >= 0) {
          scaredFlash(ledstrip[i], numFlashes);
        }
        //red "sparkling"
        if (numFlashes <= 0) {
          terrified(ledstrip[i]);
        }
        //reset
        if (numFlashes < -10) {
          numFlashes = 8;
        }
      }
      timeSinceLastFlash = millis();
      numFlashes--;
    }
  }

  if ((millis() - timeSinceLastChange) > desiredChangeRate) {   //Regulate frequency of LED update
    for (int i = 0; i < NUM_LEDS; i++) {
      //TIMID
      if (dist == 1 && vel == 0) {
        timid(ledstrip[i]);
      }

      //TRUSTING
      if (dist == 0 && vel == 1) {
        trusting(ledstrip[i]);
      }
    }

    //STOIC
    if (ultraVal == 0 && micVal == 0) {
      //make sure red and green can't stay on
      for (int i; i < NUM_LEDS; i++) {
        ledstrip[i].g = 0;
        ledstrip[i].r = 0;
      }
      //blue starry effect
      for (int i = 0; i < 76; i = i + 2) {
        ledstrip[i] = dim8_raw((exp(sin(remainingInt[i] / fadingInt[i] * PI)) - 0.36787944) * 108.0);
        ledstrip[i].r = 0;
        //Restart
        if (!--remainingInt[i]) {
          fadingInt[i] = random(10, 20);
          remainingInt[i] = fadingInt[i];
        }
      }
    }
    timeSinceLastChange = millis();   //Restart the timer
  }
  FastLED.show();
}

void terrified(CRGB & led) {
  //Makes the wisp quickly flash red, might go well with the shivering of terrified
  int bright = random8(100, 200);
  int brightG = random8(80);

  int valR = dim8_raw(random(100, 200));
  //Uncomment for orange flashes
  int valG = dim8_raw(random(0, 80));

  led.r = scale8(valR, bright);
  led.g = 0; //scale8(valG, bright);  //uncomment for the orange flashes
  led.b = 0;

}

void trusting(CRGB & led) {
  //Makes the wisp "sparkle" with random RGB colors, might go well with the shivering of terrified
  int brightR = random8(150);
  int brightG = random8(150);
  int brightB = random8(150);

  int valR = dim8_raw(random8(255));
  int valG = dim8_raw(random8(255));
  int valB = dim8_raw(random8(255));

  led.r = scale8(valR, brightR);
  led.g = scale8(valG, brightG);
  led.b = scale8(valB, brightB);
}

void scaredFlash(CRGB & led, int numFlashes) {
  //LEDs flash red
  if (numFlashes % 2 == 0) {
    led = CRGB::Black;
  }
  else if (numFlashes % 2 != 0) {
    led = CRGB::Red;
  }
}

void timid(CRGB & led) {
  //Lerp continuously between red and purple/blue
  float bright = dim8_raw((exp(sin(millis() / 2000.0 * PI)) - 0.36787944) * 108.0);
  led.b = scale8(150, bright);
  led.g = 0;
  led.r = 50;
}
