// Includes
#include <avr/power.h>
#include "Combo.h"
#include "Direction.h"

// Analog Pins
int xPin = 1;
int yPin = 2;

// Digital Pins
int lockPin = 3;
int greenLED = 1;
int redLED = 0;

/**
 * Direction instances.
 * Coordinates are made into constants so they can be adjusted easily.
 * ------ XY Matrix 3v ------
 *   L       C        R
 *  Area    Area     Area
 * 0-159  241-387  469-628
 * 
 * ------ XY Matrix 5v ------
 *   L       C        R
 *  Area    Area     Area
 * 0-204  414-614  828-1028
 */
// X Axis
const int leftOfLeftX = 0;
const int rightOfLeftX = 159;
const int leftOfCenterX = 241;
const int rightOfCenterX = 387;
const int leftOfRightX = 469;
const int rightOfRightX = 628;

// Y Axis
const int topOfTopY = 628;
const int bottomOfTopY = 469;
const int topOfMiddleY = 387;
const int bottomOfMiddleY = 241;
const int topOfBottom = 159;
const int bottomOfBottom = 0;

// Top Directions
Direction TopLeft(leftOfLeftX, bottomOfTopY, rightOfLeftX, topOfTopY, 0);
Direction TopCenter(leftOfCenterX, bottomOfTopY - 25, rightOfCenterX, topOfTopY, 1);
Direction TopRight(leftOfRightX, bottomOfTopY, rightOfRightX, topOfTopY, 2);

// Center Directions
Direction CenterLeft(leftOfLeftX, bottomOfMiddleY, rightOfLeftX, topOfMiddleY, 3);
Direction CenterCenter(leftOfCenterX - 50, bottomOfMiddleY - 50, rightOfCenterX + 50, topOfMiddleY + 25, 4);
Direction CenterRight(leftOfRightX, bottomOfMiddleY, rightOfRightX, topOfMiddleY, 5);

// Bottom Directions
Direction BottomLeft(leftOfLeftX, bottomOfBottom, rightOfLeftX, topOfBottom, 6);
Direction BottomCenter(leftOfCenterX, bottomOfBottom, rightOfCenterX, topOfBottom, 7);
Direction BottomRight(leftOfRightX, bottomOfBottom, rightOfRightX, topOfBottom, 8);

// Default Values
int xVal = 0;
int yVal = 0;
bool isFadingToGreen = false;
bool isFadingToRed = false;
bool isListeningToJoystick = true;
int fadeLength = 5000;
long fadeTime = 0;
Combo ComboCollection;
int currentDirection;
int openComboLength = 3;
int comboResetTime = 6000;
int comboTime = 0;
bool isComboCountingDown = false;
bool check = false;

/**
 * Initial setup
 */
void setup() {
  // Set the clock to 16MHz.
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  
  // Initialize pins.
  pinMode(lockPin, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(2, INPUT);
  pinMode(4, INPUT);

  // Establish the default direction of the joystick.
  currentDirection = CenterCenter.id();

  // Register the combo that activate the solenoid.
  int openCombo[openComboLength] = {TopCenter.id(), TopCenter.id(), TopCenter.id()};
  ComboCollection.init(openCombo, openComboLength);
}

/**
 * Good ol lop function. Loopy loop loop.
 */
void loop() {
  // Sometimes we want to listen for joystick inputs, sometimes we don't.
  if (isListeningToJoystick) {
    recordJoystick();
  }

  // Using flag to perform fade so that we don't need to use a while loop.
  if (isFadingToGreen) {
    fadeToGreen();
  }

  // Using flag to perform fade so that we don't need to use a while loop.
  if (isFadingToRed) {
    fadeToRed();
  }

  // When an input is received, but the combo isn't complete, execute timeout operations.
  if (isComboCountingDown) {
    comboTimeout();
  }
}

/**
 * When an input is received, begin combo timeout countdown.
 */
void activateComboTimeout() {
  isComboCountingDown = true;
  comboTime = comboResetTime;
}

/**
 * Stops and resets the combo timeout operation.
 */
void haltComboTimeout() {
  isComboCountingDown = false;
  comboTime = 0;
}

/**
 * Counts down after an input. If time is up, it resets itself and the combo,
 * and registers the combo as invalid.
 */
void comboTimeout() {
  comboTime--;
  if (comboTime <= 0) {
    haltComboTimeout();
    ComboCollection.resetActiveCombo();
    blinkLight(redLED, 6, true, 2000);
  }
}

/**
 * Listens to the joystick for changes.
 */
void recordJoystick() {
  xVal = analogRead(xPin);
  yVal = analogRead(yPin);

  // First check to see if it is no longer centered.
  bool isCentered = CenterCenter.checkCoordinate(xVal, yVal);

  // If it's not in the center...
  if (isCentered == false) {
    bool isInDirection = false;
    int newDirection;
    
    // Find the direction it's in...
    if (yVal >= bottomOfTopY) {
      // Check the top 3 boxes.
      Direction topDirections[3] = {TopLeft, TopCenter, TopRight};
      
      for (int i = 0; i < 3; i++) {
        Direction dir = topDirections[i];
        bool foundDirection = dir.checkCoordinate(xVal, yVal);
        
        if (foundDirection == true) {
          isInDirection = true;
          newDirection = dir.id();
          break;
        }
      }
    } else if (yVal >= bottomOfMiddleY) {
      // Check the 2 center boxes that is not the dead center one.
      Direction centerDirections[2] = {CenterLeft, CenterRight};
      
      for (int i = 0; i < 2; i++) {
        Direction dir = centerDirections[i];
        bool foundDirection = dir.checkCoordinate(xVal, yVal);
        
        if (foundDirection == true) {
          isInDirection = true;
          newDirection = dir.id();
          break;
        }
      }
    } else {
      // Check the bottom 3 boxes.
      Direction bottomDirections[3] = {BottomLeft, BottomCenter, BottomRight};
      
      for (int i = 0; i < 2; i++) {
        Direction dir = bottomDirections[i];
        bool foundDirection = dir.checkCoordinate(xVal, yVal);
        
        if (foundDirection == true) {
          isInDirection = true;
          newDirection = dir.id();
          break;
        }
      }
    }

    // If the joystick is not in a dead-zone and it's direction is not the same...
    if (isInDirection == true && newDirection != currentDirection) {
      currentDirection = newDirection;
      int comboSize = ComboCollection.addDirectionToCombo(newDirection);

      // Check the combo for validity...
      if (comboSize == openComboLength) {
        haltComboTimeout();
        checkCombo();
      } else {
        activateComboTimeout();
        blinkLight(redLED, 1, true, 1000);
      }
    }
  } else if (currentDirection != CenterCenter.id()) {
    currentDirection = CenterCenter.id();
  }
}

/**
 * Checks the combo against the registered open combo.
 * If it's a match, activate the solenoid.
 * If not, tell user it's invalid.
 */
void checkCombo() {
  isListeningToJoystick = false;
  bool isComboValid = ComboCollection.checkCombo();
  ComboCollection.resetActiveCombo();

  if (isComboValid == true) {
    unlockDoor();
  } else {
    blinkLight(redLED, 6, true, 2000);
    isListeningToJoystick = true;
  }
}

/**
 * Unlocks the door by activating the solenoid.
 * Illuminates the green LEDs.
 */
void unlockDoor() {
  isFadingToGreen = true;
  digitalWrite(lockPin, HIGH);
}

/**
 * Deactivates the solenoid.
 * Fades the green LEDs.
 * (Used to re-activate an ambient red glow. No longer the case.)
 */
void lockDoor() {
  isFadingToRed = true;
  digitalWrite(lockPin, LOW);
}

/**
 * Fades in the Green LEDs over time.
 * (Used to fade out the red ambient glow, which is currently innactive.)
 */
void fadeToGreen() {
  fadeTime++;
  int greenVal = 255 - (255 * cos(2 * (PI / fadeLength) * fadeTime));
//  int redVal = 255 - (255 * sin(2 * (PI / fadeLength) * fadeTime));
  
  analogWrite(greenLED, greenVal);
//  analogWrite(redLED, redVal);

  if (greenVal >= 255) {
    resetFadeValues();
    delay(3000);
    lockDoor();
  }
}

/**
 * Used to fade out green, and fade in the red ambient glow.
 * Now just fades out the Green LEDs.
 */
void fadeToRed() {
  fadeTime++;
  int greenVal = 255 - (255 * sin(2 * (PI / fadeLength) * fadeTime));
//  int redVal = 255 - (255 * cos(2 * (PI / fadeLength) * fadeTime));
  
  analogWrite(greenLED, greenVal);
//  analogWrite(redLED, redVal);

  if (greenVal <= 0) {
    resetFadeValues();
    isListeningToJoystick = true;
  }
}

/**
 * Performs a blink action on an LED a number of times at variable blink durations.
 * Can blink an LED that starts dark or illuminated.
 */
void blinkLight(int ledPin, int blinkTimes, bool isLedOff, int blinkLength) {
  // If the LED starts blinking while off...
  if (isLedOff == true) {
    for (int i = 0; i < blinkTimes; i++) {
      int ledVal = 0;
      int blinkTime = 0;
  
      // Off to on
      do {
        blinkTime++;
        ledVal = 255 - (255 * cos(2 * (PI / blinkLength) * blinkTime));
        analogWrite(ledPin, ledVal);
      } while (ledVal < 255);
  
      blinkTime = 0;
      
      // On to off
      do {
        blinkTime++;
        ledVal = 255 - (255 * sin(2 * (PI / blinkLength) * blinkTime));
        analogWrite(ledPin, ledVal);
      } while (ledVal > 0);
    }
  } else {
    // LED stats blinking while on.
    for (int i = 0; i < blinkTimes; i++) {
      int ledVal = 255;
      int blinkTime = 0;
  
      // On to off
      do {
        blinkTime++;
        ledVal = 255 - (255 * sin(2 * (PI / blinkLength) * blinkTime));
        analogWrite(ledPin, ledVal);
      } while (ledVal > 0);
  
      blinkTime = 0;
      
      // Off to on
      do {
        blinkTime++;
        ledVal = 255 - (255 * cos(2 * (PI / blinkLength) * blinkTime));
        analogWrite(ledPin, ledVal);
      } while (ledVal < 255);
    }
  }
}

/**
 * Fade values are used to track the different fade flags used in the loop function,
 * as well as the global values used during fade events.
 */
void resetFadeValues() {
  isFadingToGreen = false;
  isFadingToRed = false;
  fadeTime = 0;
}

