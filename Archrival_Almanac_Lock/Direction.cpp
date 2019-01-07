#include "Arduino.h"
#include "Direction.h"

/**
 * Constructor
 * X and Y coordinates define a sector.
 * x1 and y1 are for bottom-left.
 * x2 and y2 are for top-right.
 */
Direction::Direction(int x1, int y1, int x2, int y2, int id) {
  _x1 = x1;
  _x2 = x2;
  _y1 = y1;
  _y2 = y2;
  _id = id;
}

/**
 * Takes an X and Y coordinate and checks to see if it's within it's own bounds.
 * Returns a boolean based on whether it's within this Direction's bounds.
 */
bool Direction::checkCoordinate(int x, int y) {
  bool isInSector = false;

  if (x >= _x1 && x <= _x2 && y >= _y1 && y <= _y2) {
    isInSector = true;
  }

  return isInSector;
}

/**
 * Returns the ID of this Direction.
 */
int Direction::id() {
  return _id;
}

