/*
 ************ MAKEY MAKEY CUSTOM FIRMWARE ************
 * 
 * Adapted from:
 * 
 * - https://github.com/sparkfun/MaKeyMaKey
 * - https://github.com/DavidRieman/MaKeyMaKey_Flexible/
 * 
 */

////////////////////////
// DEFINED CONSTANTS////
////////////////////////

#define BUFFER_LENGTH     3     // 3 bytes gives us 24 samples
#define NUM_INPUTS        13    // 6A + 6D + 1
#define OPTIONS_PER_INPUT 4     // CTRL + ALT + SHIFT + 'letter'
#define KEYS_PER_INPUT    4     // ditto
#define TARGET_LOOP_TIME  744   // (1/56 seconds) / 24 samples = 744 microseconds per sample 

#include <Keyboard.h>
#include "settings.h"

/////////////////////////
// STRUCT ///////////////
/////////////////////////

typedef struct {
  byte pinNumber;
  int keyCode[KEYS_PER_INPUT];
  byte measurementBuffer[BUFFER_LENGTH]; 
  boolean oldestMeasurement;
  byte bufferSum;
  boolean pressed;
  boolean prevPressed;
  boolean isKeyboardKey;
  boolean isReverseInput;
} 

MakeyMakeyInput;

MakeyMakeyInput inputs[NUM_INPUTS];

///////////////////////////////////
// VARIABLES //////////////////////
///////////////////////////////////

int bufferIndex = 0;
byte byteCounter = 0;
byte bitCounter = 0;

int pressThreshold;
int releaseThreshold;
boolean inputChanged;

// Pin Numbers
int pinNumbers[NUM_INPUTS] = {
  13,                      // top of makey makey board (left)
  5, 4, 3, 2, 1, 0,        // left side of female header, KEYBOARD
  23, 22, 21, 20, 19, 18   // right side of female header, MOUSE
};

// input status LED pin numbers
const int inputLED_a = 9;
const int inputLED_b = 10;
const int inputLED_c = 11;
const int outputK = 14;
const int outputM = 16;
byte ledCycleCounter = 0;

// timing
int loopTime = 0;
int prevTime = 0;
int loopCounter = 0;

// LED
int buttonLed = 6;
int buttonBrightness = 0;
int buttonFadeAmount = 1;
int buttonDelay = 6;

///////////////////////////
// FUNCTIONS //////////////
///////////////////////////
void initializeArduino();
void initializeInputs();
void updateMeasurementBuffers();
void updateBufferSums();
void updateBufferIndex();
void updateInputStates();
void addDelay();
void cycleLEDs();
void danceLeds();
void updateOutLEDs();

//////////////////////
// SETUP /////////////
//////////////////////
void setup() 
{
  initializeArduino();
  initializeInputs();
  danceLeds();
}

////////////////////
// MAIN LOOP ///////
////////////////////
void loop() 
{
  updateMeasurementBuffers();
  updateBufferSums();
  updateBufferIndex();
  updateInputStates();
  cycleLEDs();
  updateOutLEDs();
  addDelay();
  fadeButton();
}

//////////////////////////
// INITIALIZE ARDUINO
//////////////////////////
void initializeArduino() {
#ifdef DEBUG
  Serial.begin(9600);  // Serial for debugging
#endif

  /* Set up input pins 
   DEactivate the internal pull-ups, since we're using external resistors */
  for (int i=0; i<NUM_INPUTS; i++)
  {
    pinMode(pinNumbers[i], INPUT);
    digitalWrite(pinNumbers[i], LOW);
  }

  pinMode(inputLED_a, INPUT);
  pinMode(inputLED_b, INPUT);
  pinMode(inputLED_c, INPUT);
  digitalWrite(inputLED_a, LOW);
  digitalWrite(inputLED_b, LOW);
  digitalWrite(inputLED_c, LOW);

  pinMode(outputK, OUTPUT);
  pinMode(outputM, OUTPUT);
  digitalWrite(outputK, LOW);
  digitalWrite(outputM, LOW);

  pinMode(buttonLed, OUTPUT);

#ifdef DEBUG
  delay(4000); // allow us time to reprogram in case things are freaking out
#endif

  Keyboard.begin();
}

///////////////////////////
// INITIALIZE INPUTS
///////////////////////////
void initializeInputs() {

  float thresholdPerc = SWITCH_THRESHOLD_OFFSET_PERC;
  float thresholdCenterBias = SWITCH_THRESHOLD_CENTER_BIAS/50.0;
  float pressThresholdAmount = (BUFFER_LENGTH * 8) * (thresholdPerc / 100.0);
  float thresholdCenter = ( (BUFFER_LENGTH * 8) / 2.0 ) * (thresholdCenterBias);
  pressThreshold = int(thresholdCenter + pressThresholdAmount);
  releaseThreshold = int(thresholdCenter - pressThresholdAmount);

  for (int i=0; i<NUM_INPUTS; i++) {
    inputs[i].pinNumber = pinNumbers[i];
 
    for (int k=0; k<KEYS_PER_INPUT; k++) {
      // Track all the key(s) that are to be triggered by the input 'i'.
      int keyCode = keyCodesAndOptions[i][k];
      inputs[i].keyCode[k] = keyCode;
      
      if (InputIsKeyboardKey(keyCode)) {
        inputs[i].isKeyboardKey = true;
      }
    }

    for (int j=0; j<BUFFER_LENGTH; j++) {
      inputs[i].measurementBuffer[j] = 0;
    }
  }
}


//////////////////////////////
// UPDATE MEASUREMENT BUFFERS
//////////////////////////////
void updateMeasurementBuffers() {

  for (int i=0; i<NUM_INPUTS; i++) {

    // store the oldest measurement, which is the one at the current index,
    // before we update it to the new one 
    // we use oldest measurement in updateBufferSums
    byte currentByte = inputs[i].measurementBuffer[byteCounter];
    inputs[i].oldestMeasurement = (currentByte >> bitCounter) & 0x01; 

    boolean newMeasurement;

    // Make the new measurement, by reading the input directly.
    newMeasurement = digitalRead(inputs[i].pinNumber);
    
    // invert so that true means the switch is closed
    newMeasurement = !newMeasurement; 

    // store it    
    if (newMeasurement) {
      currentByte |= (1<<bitCounter);
    } 
    else {
      currentByte &= ~(1<<bitCounter);
    }
    inputs[i].measurementBuffer[byteCounter] = currentByte;
  }
}

///////////////////////////
// UPDATE BUFFER SUMS
///////////////////////////
void updateBufferSums() {

  // the bufferSum is a running tally of the entire measurementBuffer
  // add the new measurement and subtract the old one

  for (int i=0; i<NUM_INPUTS; i++) {
    byte currentByte = inputs[i].measurementBuffer[byteCounter];
    boolean currentMeasurement = (currentByte >> bitCounter) & 0x01; 
    if (currentMeasurement) {
      inputs[i].bufferSum++;
    }
    if (inputs[i].oldestMeasurement) {
      inputs[i].bufferSum--;
    }
  }  
}

///////////////////////////
// UPDATE BUFFER INDEX
///////////////////////////
void updateBufferIndex() {
  bitCounter++;
  if (bitCounter == 8) {
    bitCounter = 0;
    byteCounter++;
    if (byteCounter == BUFFER_LENGTH) {
      byteCounter = 0;
    }
  }
}

int InputIsKeyboardKey(int inputCode) {
  return inputCode > 0;
}

void PressKeyboard(int inputIndex) {
  MakeyMakeyInput input = inputs[inputIndex];
  for (int k=0; k<KEYS_PER_INPUT; k++)
  {
    int keyCode = input.keyCode[k];
    if (InputIsKeyboardKey(keyCode))
    {
      if (!input.isReverseInput) {
        Keyboard.press(keyCode);
      } else {
        Keyboard.release(keyCode);
      }
    }
  }
}

void ReleaseKeyboard(int inputIndex)
{
  MakeyMakeyInput input = inputs[inputIndex];
  for (int k=0; k<KEYS_PER_INPUT; k++)
  {
    int keyCode = input.keyCode[k];
    if (InputIsKeyboardKey(keyCode))
    {
      if (!input.isReverseInput) {
        Keyboard.release(keyCode);
      } else {
        Keyboard.press(keyCode);
      }
    }
  }
}

///////////////////////////
// UPDATE INPUT STATES
///////////////////////////
void updateInputStates() {
  for (int i=0; i<NUM_INPUTS; i++) {
    inputs[i].prevPressed = inputs[i].pressed; // store previous pressed state (only used for mouse buttons)
    if (inputs[i].pressed) {
      if (inputs[i].bufferSum < releaseThreshold) {  
        if (inputs[i].isKeyboardKey) {
          inputs[i].pressed = false;
          ReleaseKeyboard(i);
        }
      }
    } 
    else if (!inputs[i].pressed) {
      if (inputs[i].bufferSum > pressThreshold) {  // input becomes pressed
        if (inputs[i].isKeyboardKey) {
          inputs[i].pressed = true;
          PressKeyboard(i);
        }
      }
    }
  }
}

///////////////////////////
// ADD DELAY
///////////////////////////
void addDelay() {
  loopTime = micros() - prevTime;
  if (loopTime < TARGET_LOOP_TIME) {
    int wait = TARGET_LOOP_TIME - loopTime;
    delayMicroseconds(wait);
  }

  prevTime = micros();
}

///////////////////////////
// CYCLE LEDS
///////////////////////////
void cycleLEDs() {
  pinMode(inputLED_a, INPUT);
  pinMode(inputLED_b, INPUT);
  pinMode(inputLED_c, INPUT);
  digitalWrite(inputLED_a, LOW);
  digitalWrite(inputLED_b, LOW);
  digitalWrite(inputLED_c, LOW);

  ledCycleCounter++;
  ledCycleCounter %= 6;

  if ((ledCycleCounter == 0) && inputs[0].pressed) {
    pinMode(inputLED_a, INPUT);
    digitalWrite(inputLED_a, LOW);
    pinMode(inputLED_b, OUTPUT);
    digitalWrite(inputLED_b, HIGH);
    pinMode(inputLED_c, OUTPUT);
    digitalWrite(inputLED_c, LOW);
  }
  if ((ledCycleCounter == 1) && inputs[1].pressed) {
    pinMode(inputLED_a, OUTPUT);
    digitalWrite(inputLED_a, HIGH);
    pinMode(inputLED_b, OUTPUT);
    digitalWrite(inputLED_b, LOW);
    pinMode(inputLED_c, INPUT);
    digitalWrite(inputLED_c, LOW);
  }
  if ((ledCycleCounter == 2) && inputs[2].pressed) {
    pinMode(inputLED_a, OUTPUT);
    digitalWrite(inputLED_a, LOW);
    pinMode(inputLED_b, OUTPUT);
    digitalWrite(inputLED_b, HIGH);
    pinMode(inputLED_c, INPUT);
    digitalWrite(inputLED_c, LOW);
  }
  if ((ledCycleCounter == 3) && inputs[3].pressed) {
    pinMode(inputLED_a, INPUT);
    digitalWrite(inputLED_a, LOW);
    pinMode(inputLED_b, OUTPUT);
    digitalWrite(inputLED_b, LOW);
    pinMode(inputLED_c, OUTPUT);
    digitalWrite(inputLED_c, HIGH);
  }
  if ((ledCycleCounter == 4) && inputs[4].pressed) {
    pinMode(inputLED_a, OUTPUT);
    digitalWrite(inputLED_a, LOW);
    pinMode(inputLED_b, INPUT);
    digitalWrite(inputLED_b, LOW);
    pinMode(inputLED_c, OUTPUT);
    digitalWrite(inputLED_c, HIGH);
  }
  if ((ledCycleCounter == 5) && inputs[5].pressed) {
    pinMode(inputLED_a, OUTPUT);
    digitalWrite(inputLED_a, HIGH);
    pinMode(inputLED_b, INPUT);
    digitalWrite(inputLED_b, LOW);
    pinMode(inputLED_c, OUTPUT);
    digitalWrite(inputLED_c, LOW);
  }
}

///////////////////////////
// DANCE LEDS
///////////////////////////
void danceLeds()
{
  int delayTime = 50;
  int delayTime2 = 100;

  // CIRCLE
  for(int i=0; i<4; i++)
  {
    // UP
    pinMode(inputLED_a, INPUT);
    digitalWrite(inputLED_a, LOW);
    pinMode(inputLED_b, OUTPUT);
    digitalWrite(inputLED_b, HIGH);
    pinMode(inputLED_c, OUTPUT);
    digitalWrite(inputLED_c, LOW);
    delay(delayTime);

    // RIGHT
    pinMode(inputLED_a, INPUT);
    digitalWrite(inputLED_a, LOW);
    pinMode(inputLED_b, OUTPUT);
    digitalWrite(inputLED_b, LOW);
    pinMode(inputLED_c, OUTPUT);
    digitalWrite(inputLED_c, HIGH);
    delay(delayTime);

    // DOWN
    pinMode(inputLED_a, OUTPUT);
    digitalWrite(inputLED_a, HIGH);
    pinMode(inputLED_b, OUTPUT);
    digitalWrite(inputLED_b, LOW);
    pinMode(inputLED_c, INPUT);
    digitalWrite(inputLED_c, LOW);
    delay(delayTime);

    // LEFT
    pinMode(inputLED_a, OUTPUT);
    digitalWrite(inputLED_a, LOW);
    pinMode(inputLED_b, OUTPUT);    
    digitalWrite(inputLED_b, HIGH);
    pinMode(inputLED_c, INPUT);
    digitalWrite(inputLED_c, LOW);
    delay(delayTime);    
  }    

  // WIGGLE
  for(int i=0; i<4; i++)
  {
    // SPACE
    pinMode(inputLED_a, OUTPUT);
    digitalWrite(inputLED_a, HIGH);
    pinMode(inputLED_b, INPUT);
    digitalWrite(inputLED_b, LOW);
    pinMode(inputLED_c, OUTPUT);
    digitalWrite(inputLED_c, LOW);
    delay(delayTime2);    

    // CLICK
    pinMode(inputLED_a, OUTPUT);
    digitalWrite(inputLED_a, LOW);
    pinMode(inputLED_b, INPUT);
    digitalWrite(inputLED_b, LOW);
    pinMode(inputLED_c, OUTPUT);
    digitalWrite(inputLED_c, HIGH);
    delay(delayTime2);    
  }
}

void updateOutLEDs()
{
  boolean keyPressed = 0;

  for (int i=0; i<NUM_INPUTS; i++)
  {
    if (inputs[i].pressed)
    {
      if (inputs[i].isKeyboardKey)
      {
        keyPressed = 1;
#ifdef DEBUG
        Serial.print("Key ");
        Serial.print(i);
        Serial.println(" pressed");
#endif
      }
    }
  }

  if (keyPressed)
  {
    digitalWrite(outputK, HIGH);
    TXLED1;
  }
  else
  {
    digitalWrite(outputK, LOW);
    TXLED0;
  }
}

// 
// Fade big red button in and out
//
void fadeButton()
{
  analogWrite(buttonLed, buttonBrightness);
  if (buttonDelay-- == 0) {
    buttonDelay = 6;
    buttonBrightness = buttonBrightness + buttonFadeAmount;
    if (buttonBrightness <= 0 || buttonBrightness >= 255 ) {
      buttonFadeAmount = -buttonFadeAmount;
    }
  }
}
