#include <FastLED.h>

//Define number of LEDs per strip
#define NUM_LEDS_PART_1 20
#define NUM_LEDS_PART_2 24
#define NUM_LEDS_PART_3 20
#define NUM_LEDS_PART_4 19

//Define number of LEDs in overall strip
#define NUM_LEDS (NUM_LEDS_PART_1 + NUM_LEDS_PART_2) // + NUM_LEDS_PART_3 + NUM_LEDS_PART_4)

//Define global brightness
#define BRIGHTNESS      64

//Define strip object
CRGB ledstrip[NUM_LEDS_PART_1 + NUM_LEDS_PART_2]; // + NUM_LEDS_PART_3 + NUM_LEDS_PART_4];

bool ascent;
bool flag = false;
bool doneBooping = true;
bool doneFlashing = false;
bool endOfStrip = false;
float timeSinceLastBoop;
int currentIteration = 0;
int numFlashes = 6;

float desiredUpdateRate = 80;
float desiredFlashRate = 150;

float timeSinceLastUpdate;
float timeSinceLastFlash;

int n;
int k;

void setup() {
  delay(3000);  //delay 3 seconds for safety purposes

  //Add LED strips
  FastLED.addLeds<NEOPIXEL, 7>(ledstrip, 0, NUM_LEDS_PART_1);
  FastLED.addLeds<NEOPIXEL, 8>(ledstrip, NUM_LEDS_PART_1, NUM_LEDS_PART_2);
  //FastLED.addLeds<NEOPIXEL, 9>(ledstrip, NUM_LEDS_PART_2, NUM_LEDS_PART_3);
  //FastLED.addLeds<NEOPIXEL, 10>(ledstrip, NUM_LEDS_PART_3, NUM_LEDS_PART_4);
  
  Serial.begin(9600); //Initialize Serial monitor
  FastLED.setBrightness(BRIGHTNESS);  //Set global brightness

}

void loop() {
  
  //TODO: scared action triggers, store time, pass into and call scare function
  //TODO: Replace with call to scare function (when it's working)

  //Main loop to update all LEDs
  if ((millis() - timeSinceLastFlash) > desiredFlashRate){
    if (numFlashes > 0) {
      for (int n = 0; n < NUM_LEDS; n++) {
        ScaredFlash(ledstrip[n]);
      }
      numFlashes--;
    }
    timeSinceLastFlash = millis();
  }
  
  //Initialize times for next loop
  if (numFlashes == 0){
    //float timeSinceLastBoop = millis();
    float timeSinceLastUpdate = millis();
    doneBooping = false;
    flag = false;
    numFlashes--;
  }
  
  //Part two
  if ((millis() - timeSinceLastUpdate) > desiredUpdateRate){
    if (doneBooping == false) {
      for (int k = 0; k < NUM_LEDS; k++) {
        doneBooping = BoopGraph(ledstrip[k]);
        if (k == (NUM_LEDS - 1)){
          endOfStrip = true;
          Serial.println(k);
        }
      }
      Serial.println(flag);
    }
      
    if (millis() > 25000 && millis() < 30000){
      if (ledstrip[NUM_LEDS - 1].b < 200){
        for (int n = 0; n < NUM_LEDS; n++) {
          ledstrip[n] += CRGB(0, 0, 5); //TODO: Change color to not necessarily pure blue
          Serial.println("Fade On");
        }
      }
    }
  
    if (millis() > 30000){
      for (int n = 0; n < NUM_LEDS; n++) {
        ledstrip[n] = CRGB::Black;
        Serial.println("Turn off LEDs");
      }
    }
    timeSinceLastUpdate = millis(); 
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

void ScaredFlash(CRGB& led) {
  if (led.r > 25) {
    led = CRGB::Black;
  }
  else if (led == CRGB(0, 0, 0)) {
    led = CRGB::Red;
  }
}

bool BoopGraph(CRGB& led) {
  //Colors to progress from scared to nervous
  int colorarray[] = {250, 0, 0, 
                      200, 0, 50, 
                      150, 0, 100, 
                      100, 0, 150,
                      50,  0, 200, 
                      0,   0, 250};
  
  float boopRest = millis() - timeSinceLastBoop;

  if (boopRest >= 1500){
    //If the lights have been off for the appropriate amount of time, initialize the next boop animation
      breatheOnOff(led, 40, 40, colorarray[currentIteration * 3], colorarray[currentIteration * 3 + 1], colorarray[currentIteration * 3 + 2]); //change colors to go slowly from red to blue
  }
  
  if (flag == true){
    if ((led.r + led.g + led.b) <= 0 && endOfStrip == true){
      timeSinceLastBoop = millis(); //Reset the delay timer
      flag = false;
      endOfStrip = false;
      Serial.println("Resetting");
      currentIteration++;
    }
  }

  if (currentIteration <6){
    return false;
  }
  if (currentIteration == 6){
    return true; 
  }
}



/*void scared(CRGB& led, float callTime){
/*Takes the led strip object and the time at which the wisp becomes scared, and executes the scared animations */
  /*//Initialize variables once, when function is called
  if (millis() - callTime == 0){
    int numFlashes = 5;
    float timesinceLastFlash = millis();
  }

  //Main loop to update all LEDs
  for (int n = 0; n < NUM_LEDS; n++) {
    if (numFlashes > 0) {
      Flash(led[n], numFlashes, 100);
      Serial.println("flash");   
      }
      //Initialize times for next loop
    if (numFlashes == 0){
      float timeSinceLastBoop = millis();
      float timeSinceLastUpdate = millis();
      bool doneBooping = false;    //If I change this value in a function will it be updated automatically or do I need to return it in the function?
    }
    
    else if (millis() - timeSinceLastUpdate >= desiredUpdateRate){
      if (doneBooping == false && millis() >= callTime + 6500) {
        BoopGraph(ledstrip[n]);
        Serial.println("Boop");
      }
      
      else if (millis() > callTime + 25000 && millis() < callTime + 30000){
        if (ledstrip[NUM_LEDS - 1].b < 200){
          ledstrip[n] += CRGB(0, 0, 5); //TODO: Change color to not necessarily pure blue
          Serial.println("Fade On");
        }
      }
  
      else if (millis() > callTime + 30000){
        ledstrip[n] = CRGB::Black;
        Serial.println("Turn off LEDs");
      }
    } 

  }
  
} */
