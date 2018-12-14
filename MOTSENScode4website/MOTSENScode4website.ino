#include <SpeedyStepper.h>
//************** CODE FOR ARDUINO NUMBER ONE OF THE WISP. PLUGS INTO STEPPER AND SENSOR******
const int MOTOR_STEP_PIN = 9;
const int MOTOR_DIRECTION_PIN = 8;
const int stepperSpeed = 100 * 16;  //Configured for 8x microstepping
const int maxSteps = 25 * 16;   //Where 25 * 16 == fully open and 0 == fully closed

//For distance sensor
const int trigPin = 11; 
const int echoPin = 10;

//For sending commands to Arduino 2  
const int ultraPin = 12;
const int micPin = 13;

//Cutoff values for distance and velocity
const int distThresh = 40;
const int velThresh = 200;

//variables for calculating distance and velocity
long duration;
float distance, vel;
unsigned long lastVT;
int dT = 100;
long lastDist = 0;

//triggers for timid and trust behavior
bool timTrig = false;
bool trustTrig = false;

//timid variables
unsigned long timidTime;
unsigned long timidInt = 10000;

//motor variables
int currentSteps = maxSteps;  //For tracking current motor position
bool openFlag = true;
bool closedFlag = false;
bool peeked = false;
unsigned long peekTime;
unsigned long peekInt = 4000;
SpeedyStepper stepper;  //Create stepper object

void setup()
{
  //Set up pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ultraPin, OUTPUT);
  pinMode(micPin, OUTPUT);
  Serial.begin(9600);   //Initialize Serial monitor
  stepper.connectToPins(MOTOR_STEP_PIN, MOTOR_DIRECTION_PIN);   //connect stepper object
}


void loop()
{
  //Reset default stepper speed and acceleration
  stepper.setSpeedInStepsPerSecond(stepperSpeed);
  stepper.setAccelerationInStepsPerSecondPerSecond(stepperSpeed);
  
  //Check current position of motor, set flags accordingly
  if (currentSteps >= maxSteps) {
    openFlag = true;
    closedFlag = false;
  }
  if (currentSteps <= 0) {
    closedFlag = true;
    openFlag = false;
  }
  if (currentSteps > 0 && currentSteps < maxSteps) {
    openFlag = false;
    closedFlag = false;
  }

  //Collect & process distance sensor data
  if (millis() - lastVT >= dT) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    duration = pulseIn(echoPin, HIGH);
    distance = (duration / 2) / 29.1;
    float diff = distance - lastDist;
    vel = abs(diff);  //calculate the "velocity" for every dT
    if (abs(distance - lastDist) >= velThresh) {
      Serial.println("SO FAST.");
    }
    lastDist = distance; //save current distance for next iteration
    lastVT = millis();
  }
//*************************************   BEHAVIORS   ******************************************
  //STOIC:
  if (distance > distThresh && vel < velThresh) {
    digitalWrite(ultraPin, LOW);  //send signal to Arduino 2
    digitalWrite(micPin, LOW);
    timTrig = false;
    trustTrig = false;
 // **** STOIC ****
    // Check if moving is safe
    if (!openFlag) {
      stepper.moveToPositionInSteps(maxSteps - currentSteps); //Move 
      if (stepper.motionComplete()) {
        currentSteps += (maxSteps - currentSteps);  //Update the current motor position
      }
    }
  }
//****** END STOIC ******

  //TIMID INITIALIZATION
  if (distance < distThresh && vel < velThresh && timTrig == true) {
    timidTime = millis();   //initialize timid counter
    timTrig = false;
  }
  
  //***** TRUSTING *******
  if (trustTrig == true && millis() - timidTime > timidInt) {
    //Write to Arduino 2
    digitalWrite(ultraPin, LOW);
    digitalWrite(micPin, HIGH);
    timTrig = false;

    //Check if safe to move motor
    int trustInt = 10 * 16;
    if (currentSteps + trustInt <= maxSteps && !openFlag) {
      stepper.moveToPositionInSteps(currentSteps + trustInt);
      if (stepper.motionComplete()) {
        currentSteps += trustInt; //Update current position
      }
    }
  }
  //**** END TRUSTING *** 
  
  //***** TIMID *****
  if (millis() - timidTime < timidInt) {
    if (distance < distThresh && vel < velThresh) {
      //Write to Arduino 2
      digitalWrite(ultraPin, HIGH);
      digitalWrite(micPin, LOW);
      trustTrig = true;
      //Check if motor can move
      if (currentSteps + 10 * 16 <= maxSteps && !openFlag) {
        if (peeked == false) {
          peekTime = millis();
          
          //Opens for slightly for peekInt seconds
          if (millis() - peekTime < peekInt) {
            stepper.setSpeedInStepsPerSecond(50 * 16);
            stepper.setAccelerationInStepsPerSecondPerSecond(50 * 16);
            stepper.moveToPositionInSteps(currentSteps + 10 * 16);
            if (stepper.motionComplete()) {
              currentSteps += 10 * 16;  //update position of motor
              peeked = true;
            }
          }
        }

        //Closes after peeking
        if (millis() - peekTime >= peekInt) {
          if (peeked) {
            stepper.moveToPositionInSteps(currentSteps - (10 * 16));
            if (stepper.motionComplete()) {
              currentSteps += -(10 * 16);  //update position
              peeked = false;
              peekTime = millis();  //reset timer
            }
          }
        }
      }
    }
  }
//****** END TIMID *****

  //****** SCARED *******
  if (distance <= 10 or vel > velThresh) {   
    digitalWrite(ultraPin, HIGH);
    digitalWrite(micPin, HIGH);
    if (!closedFlag) {
      stepper.moveToPositionInSteps(-maxSteps);
      if (stepper.motionComplete()) {
        currentSteps += -maxSteps;
      }
    }
    delay(5000);
    timTrig = true;
    trustTrig = false;
  }
  //********* END SCARED ******
}

