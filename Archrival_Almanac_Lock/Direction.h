#ifndef Direction_h
#define Direction_h

#include "Arduino.h"

class Direction {
  public:
    Direction(int x1, int y1, int x2, int y2, int id);
    bool checkCoordinate(int x, int y);
    int id();
  private:
    int _x1;
    int _x2;
    int _y1;
    int _y2;
    int _id;
};

#endif

