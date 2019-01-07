#ifndef Combo_h
#define Combo_h
#define MAX_INPUTS 3

#include "Arduino.h"

class Combo {
  public:
    Combo();
    void init(int* openCombo, int openComboLength);
    int addDirectionToCombo(int comboId);
    void resetActiveCombo();
    bool checkCombo();
    int getOpenComboLength();
  private:
    int _openCombo[MAX_INPUTS];
    int _openComboLength;
    int _activeCombo[MAX_INPUTS];
    int _comboSize;
};

#endif

