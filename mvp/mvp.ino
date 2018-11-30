#include <FastLED.h>  //LED stuff
#include <SpeedyStepper.h>  //Motor stuff

//Define pins for Motors
#define MOTOR_STEP_PIN 3
#define MOTOR_DIRECTION_PIN 4
#define trigPin 13
#define echoPin 12

//Define number of LEDs per strip
#define NUM_LEDS_PART_1 20
#define NUM_LEDS_PART_2 24
#define NUM_LEDS_PART_3 20
#define NUM_LEDS_PART_4 16
#define NUM_LEDS_PART_5 12

//Define number of LEDs in overall strip
#define NUM_LEDS (NUM_LEDS_PART_1 + NUM_LEDS_PART_2 + NUM_LEDS_PART_3 + NUM_LEDS_PART_4 + NUM_LEDS_PART_5)
#define BRIGHTNESS      64  //Define global brightness

//Define strip object
CRGB ledstrip[NUM_LEDS_PART_1 + NUM_LEDS_PART_2 + NUM_LEDS_PART_3 + NUM_LEDS_PART_4 + NUM_LEDS_PART_5];

//Define motor object
SpeedyStepper stepper;

//LED global variables
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

//Motor global variables
const int stepperSpeed = 50 * 16;
const int max_steps = 100 * 16;
unsigned long lastT = 0;
unsigned long scaredTime;
float last_distance = 0;
float vel = 0;
long duration, distance;
float difference;
int currentSteps = 0;
bool scaredFlag = false;
bool shutFlag = true;
unsigned long readDelay = 500;

void setup() {
  delay(3000);  //delay 3 seconds for safety purposes

  //Add LED strips
  FastLED.addLeds<NEOPIXEL, 7>(ledstrip, 0, NUM_LEDS_PART_1);
  FastLED.addLeds<NEOPIXEL, 8>(ledstrip, NUM_LEDS_PART_1, NUM_LEDS_PART_2);
  FastLED.addLeds<NEOPIXEL, 9>(ledstrip, NUM_LEDS_PART_2, NUM_LEDS_PART_3);
  FastLED.addLeds<NEOPIXEL, 10>(ledstrip, NUM_LEDS_PART_3, NUM_LEDS_PART_4);
  FastLED.addLeds<NEOPIXEL, 11>(ledstrip, NUM_LEDS_PART_4, NUM_LEDS_PART_5);


  Serial.begin(9600); //Initialize Serial monitor
  FastLED.setBrightness(BRIGHTNESS);  //Set global brightness

  //Add stepper motor pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  //pinMode(11, OUTPUT);
  stepper.connectToPins(MOTOR_STEP_PIN, MOTOR_DIRECTION_PIN);
}

void loop() {
  //Move the motors if a movement has been queued
  stepper.setSpeedInStepsPerSecond(stepperSpeed);
  stepper.setAccelerationInStepsPerSecondPerSecond(stepperSpeed);
  if (!stepper.motionComplete()) {
    stepper.processMovement();
  }

  //Collects data every READ_DELAY seconds, classifies as scared or not
  if (readDelay <= millis() - lastT) {
    Serial.print("Is scared: ");
    Serial.println(scaredFlag);
    Serial.print("Is shut: ");
    Serial.println(shutFlag);

    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    //Calculates distance between the user and sensor
    duration = pulseIn(echoPin, HIGH);
    distance = (duration / 2) / 29.1;
    Serial.print(distance);

    //Calculates velocity
    unsigned long t_elapsed = millis() - lastT;
    Serial.print(t_elapsed);
    Serial.println(" ms");
    vel = 1000 * difference / t_elapsed;  // cm/s
    lastT = millis();

    //Classifies as scared or not
    if (distance <= 20) {
      Serial.println("Just spooked");
      scaredFlag = true;
    }
    //Should only be not scared if the entire boop animation has completed
    else {
      scaredFlag = false;
    }
  }


  //SCARED BEHAVIOR ANIMATION to update all LEDs
  if (scaredFlag) {
    //If scared and not shut, close and save time that it was scared
    if (!shutFlag){
      Serial.println("Closing due to spook");
      stepper.setupMoveInSteps(-currentSteps);
      scaredTime = millis();
      shutFlag = true;
      numFlashes = 6;
    }

    if ((millis() - timeSinceLastFlash) > desiredFlashRate) {
      if (numFlashes > 0) {
        for (int n = 0; n < NUM_LEDS; n++) {
          ScaredFlash(ledstrip[n]);
        }
        numFlashes--;
      }
      timeSinceLastFlash = millis();
    }

    //Initialize times for next loop
    if (numFlashes == 0) {
      //float timeSinceLastBoop = millis();
      timeSinceLastUpdate = millis();
      doneBooping = false;
      flag = false;
      numFlashes--;
    }



  //New update rate
  if ((millis() - timeSinceLastUpdate) > desiredUpdateRate) {
    //Booping...Builds trust
    if (doneBooping == false) {
      for (int k = 0; k < NUM_LEDS; k++) {
        doneBooping = BoopGraph(ledstrip[k]);
        if (k == (NUM_LEDS - 1)) {
          endOfStrip = true;
          Serial.println(k);
        }
      }
      Serial.println(flag);
    }

    //Opening up
    if (doneBooping) {
      if (ledstrip[NUM_LEDS - 1].b < 200) {
        for (int n = 0; n < NUM_LEDS; n++) {
          ledstrip[n] += CRGB(0, 0, 5); //TODO: Change color to not necessarily pure blue
          Serial.println("Fade On");
        }
      }
    }

    //Turn off LEDs
    if (millis() > (40000 + scaredTime)) {
      for (int n = 0; n < NUM_LEDS; n++) {
        ledstrip[n] = CRGB::Black;
        Serial.println("Turn off LEDs");
      }
    }
    timeSinceLastUpdate = millis();
  }
  }
  //OPENING UP AND NOT SCARED
  else{
    if (doneBooping && shutFlag){
      stepper.setSpeedInStepsPerSecond(stepperSpeed);
      stepper.setAccelerationInStepsPerSecondPerSecond(stepperSpeed);
      stepper.setupMoveInSteps(max_steps);
      currentSteps += max_steps;
      
      //Reset all variables necessary for scared animation
      shutFlag = false;
      currentIteration = 0;
      doneBooping = false;
      endOfStrip = false;
      flag = false;      
    }
  }
  FastLED.show();
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
                      0,   0, 250
                     };

  float boopRest = millis() - timeSinceLastBoop;

  if (boopRest >= 1500) {
    //If the lights have been off for the appropriate amount of time, initialize the next boop animation
    breatheOnOff(led, 40, 40, colorarray[currentIteration * 3], colorarray[currentIteration * 3 + 1], colorarray[currentIteration * 3 + 2]); //change colors to go slowly from red to blue
  }

  if (flag == true) {
    if ((led.r + led.g + led.b) <= 0 && endOfStrip == true) {
      timeSinceLastBoop = millis(); //Reset the delay timer
      flag = false;
      endOfStrip = false;
      Serial.println("Resetting");
      currentIteration++;
    }
  }

  if (currentIteration < 6) {
    return false;
  }
  if (currentIteration == 6) {
    return true;
  }
}
