#ifndef MOVING_AVERAGE_FILTER_H
#define MOVING_AVERAGE_FILTER_H

#include <Arduino.h>

class MovingAverageFilter {
private:
  int windowSize;
  float* buffer;
  int index;
  float sum;
  bool filled;

public:
  MovingAverageFilter(int size);
  ~MovingAverageFilter();
  float filter(float input);
};

#endif
