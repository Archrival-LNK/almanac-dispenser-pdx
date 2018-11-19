#include "Arduino.h"
#include "Combo.h"

/**
 * Constructor
 */
Combo::Combo() {}

/**
 * Initializes the combo and registers the open sequence.
 */
void Combo::init(int* openCombo, int openComboLength) {
  _openComboLength = openComboLength;
  _comboSize = 0;
  
  for (int i = 0; i < _openComboLength; i++) {
    int oc = openCombo[i];
    _openCombo[i] = oc;
  }
}

/**
 * Adds a direction to the list of combo's received from input.
 */
int Combo::addDirectionToCombo(int comboId) {
  _activeCombo[_comboSize] = comboId;
  _comboSize++;

  return _comboSize;
}

/**
 * Checks the combo for validity.
 * Returns a boolean based on isComboValid.
 */
bool Combo::checkCombo() {
  bool isComboValid = true;
  for (int i = 0; i < _openComboLength; i++) {
    if (_activeCombo[i] != _openCombo[i]) {
      isComboValid = false;
      break;
    }
  }
  
  resetActiveCombo();

  return isComboValid;
}

/**
 * Resets the combo so that a new one can be recorded upon input.
 */
void Combo::resetActiveCombo() {
  _comboSize = 0;
  memset(_activeCombo, 0, sizeof(_activeCombo));
}

/**
 * Returns the length of the current combo input.
 */
int Combo::getOpenComboLength() {
  return _openComboLength;
}

