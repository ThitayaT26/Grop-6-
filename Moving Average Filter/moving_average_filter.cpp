#include "moving_average_filter.h"

MovingAverageFilter::MovingAverageFilter(int size) {
  windowSize = size;
  buffer = new float[size];
  index = 0;
  sum = 0.0;
  filled = false;
  for (int i = 0; i < windowSize; i++) {
    buffer[i] = 0.0;
  }
}

MovingAverageFilter::~MovingAverageFilter() {
  delete[] buffer;
}

float MovingAverageFilter::filter(float input) {
  sum -= buffer[index];
  buffer[index] = input;
  sum += buffer[index];
  index = (index + 1) % windowSize;
  
  if (index == 0) filled = true;
  
  int count = filled ? windowSize : index;
  return sum / count;
}
