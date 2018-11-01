#include <Servo.h>
#include <FastLED.h>

//Initialize FastLED objects
#define LED_PIN     5
#define NUM_LEDS    10
#define BRIGHTNESS  64
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define UPDATES_PER_SECOND 100
CRGB leds[NUM_LEDS];

//Define FastLED variables
CRGBPalette16 currentPalette;
TBlendType    currentBlending;

//Make servo object
Servo actuator;

//Variables that won't change
int diodeIn = A0;
int servoDelay = 50;

//Variables that will change
int angle = 0;
int sensorRead;
bool trigger = false;
unsigned long servoTime;

void setup() {
  delay(3000);  //power-up safety delay for LEDs
  //Initialize LEDs
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  //Initialize Serial connection
  Serial.begin(9600);

  //Initialize servo
  actuator.attach(9);
  actuator.write(angle);

  currentPalette = RainbowColors_p;
  currentBlending = LINEARBLEND;
}

void loop() {
  Serial.println(analogRead(diodeIn));
  Serial.println(angle);
  sensorRead = analogRead(diodeIn);
  servoTime = millis();
  static uint8_t startIndex = 0;
  startIndex = startIndex + 1;
  FillLEDsFromPaletteColors(startIndex);
  
  FastLED.show();
  //If sensor triggered, set trigger to true
  if (sensorRead <= 480){
    trigger = true;
  }
  //Turn servo gradually to 180 degrees.  
  if (trigger && angle < 180) {
      if (servoTime >= servoDelay) {
        ++angle;
        actuator.write(angle);
        servoTime = servoTime - millis(); //Reset servoTime
      }
  }

  //If servo fully turned, hold at extended position
  if (angle == 180){
    delay(500); //Get rid of this delay, possible randomize
    trigger = false;
    --angle;
    actuator.write(angle);
  }

  //Lower the actuator
  if (trigger == false && angle > 0){
      if (servoTime >= servoDelay){
        --angle;
        actuator.write(angle);
        servoTime = servoTime - millis();
    }
  }

  if (trigger == false && angle == 0){
    actuator.write(0);
  }
}

void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    uint8_t brightness = 255;
    
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }
}
