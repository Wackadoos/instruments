#include "state.h"

SensorState SensorState::empty() {
  return SensorState{
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
  };
}
