#include <SpeedyStepper.h>

const int MOTOR_STEP_PIN = 9;
const int MOTOR_DIRECTION_PIN = 8;
const int stepperSpeed = 100 * 16;
const int maxSteps = 25 * 16;
const int trigPin = 11;
const int echoPin = 10;
const int ultraPin = 12;
const int micPin = 13;
const int microInPin = 7;
const int distThresh = 40;
const int velThresh = 200; // TODO

//measurement variables
long duration;
float distance, vel;
unsigned long lastVT;
int dT = 100;
long lastDist = 0;

bool scaredFlag = false;
bool timTrig = false;
bool trustTrig = false;
//scared variables
unsigned long timeSinceScare;
unsigned long scaredInt = 3000;


//timid variables
unsigned long timidTime;
unsigned long timidInt = 10000;

//motor variables
int currentSteps = maxSteps;
bool openFlag = true;
bool closedFlag = false;
bool peeked = false;
unsigned long peekTime;
unsigned long peekInt = 4000;
SpeedyStepper stepper;



void setup()
{
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ultraPin, OUTPUT);
  pinMode(micPin, OUTPUT);
  pinMode(microInPin, INPUT);
  Serial.begin(9600);
  stepper.connectToPins(MOTOR_STEP_PIN, MOTOR_DIRECTION_PIN);
}


void loop()
{
  stepper.setSpeedInStepsPerSecond(stepperSpeed);
  stepper.setAccelerationInStepsPerSecondPerSecond(stepperSpeed);
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

  //Collect distance sensor data
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
    Serial.println(vel);
    if (abs(distance - lastDist) >= velThresh) {
      Serial.println("SO FAST.");
    }
    lastDist = distance; //save current distance for next iteration
    lastVT = millis();
  }

  //STOIC:
  if (distance > distThresh && vel < velThresh) {
    Serial.println("STOIC");
    digitalWrite(ultraPin, LOW);
    digitalWrite(micPin, LOW);
    timTrig = false;
    trustTrig = false;
    if (!openFlag) {
      Serial.println("MOTOR OPENING");
      stepper.moveToPositionInSteps(maxSteps - currentSteps);
      if (stepper.motionComplete()) {
        currentSteps += (maxSteps - currentSteps);
        Serial.println(currentSteps);
      }
    }
  }

  //TIMID
  if (distance < distThresh && vel < velThresh && timTrig == true) {
    timidTime = millis();
    timTrig = false;
  }
  //TRUSTING:
  if (trustTrig == true && millis() - timidTime > timidInt) {
    Serial.println("TRUSTING");
    digitalWrite(ultraPin, LOW);
    digitalWrite(micPin, HIGH);
    timTrig = false;
    int trustInt = 10 * 16;
    if (currentSteps + trustInt <= maxSteps && !openFlag) {
      Serial.println("TRUSTING, OPEN 75%");
      stepper.moveToPositionInSteps(currentSteps + trustInt);
      if (stepper.motionComplete()) {
        currentSteps += trustInt;
        Serial.println(currentSteps);
      }
    }
  }
  //TIMID
  if (millis() - timidTime < timidInt) {
    if (distance < distThresh && vel < velThresh) {
      Serial.println("TIMID");
      digitalWrite(ultraPin, HIGH);
      digitalWrite(micPin, LOW);
      trustTrig = true;
      if (currentSteps + 10 * 16 <= maxSteps && !openFlag) {
        if (peeked == false) {
          peekTime = millis();
          if (millis() - peekTime < peekInt) {
            Serial.println("PEEKING, OPENING");
            stepper.setSpeedInStepsPerSecond(50 * 16 && !openFlag && !closedFlag);
            stepper.setAccelerationInStepsPerSecondPerSecond(50 * 16);
            stepper.moveToPositionInSteps(currentSteps + 10 * 16);
            if (stepper.motionComplete()) {
              currentSteps += 10 * 16;
              Serial.println(currentSteps);
              peeked = true;
            }
          }

        }
        if (millis() - peekTime >= peekInt) { //TODO NEEDS DELAY
          if (peeked) {
            stepper.moveToPositionInSteps(currentSteps - (10 * 16));
            if (stepper.motionComplete()) {
              Serial.println("SHY, CLOSING");
              currentSteps += -(10 * 16);
              Serial.println(currentSteps);
              peeked = false;
              peekTime = millis();
            }
          }
        }
      }
    }
  }


  //SCARED:
  if (distance <= 10 or vel > velThresh) {    //add microphone input
    Serial.println("Just spooked");
    digitalWrite(ultraPin, HIGH);
    digitalWrite(micPin, HIGH);
    timeSinceScare = millis();
    if (!closedFlag) {
      Serial.println("motor CLOSING");
      stepper.moveToPositionInSteps(-maxSteps); ///TODO
      if (stepper.motionComplete()) {
        currentSteps += -maxSteps;
        Serial.println(currentSteps);
      }
    }
    delay(5000);
    timTrig = true;
    trustTrig = false;
  }
}

