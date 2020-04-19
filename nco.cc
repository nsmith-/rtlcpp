#include <math.h>
#include "nco.h"

bool Oscillator::sinetable_init = true;
std::array<int32_t, 256> Oscillator::sinetable;

Oscillator::Oscillator(double frequency, uint32_t sample_rate, double initial_phase) {
  if ( not sinetable_init ) {
    for (size_t i=0; i<256; ++i) {
      // 10 bit table (4 quadrants done later), 10 bit output
      sinetable[i] = std::round(512 * std::sin(i * (2*M_PI / 1024.)));
    }
    sinetable_init = true;
  }
  // 16 bits of dithering
  phase = std::round((65536 * 1024 / 2 / M_PI) * initial_phase);
  dphase = std::round(65536 * 1024 * frequency / sample_rate);
}

std::array<int32_t, 2> Oscillator::pop() {
  auto out = iq(phase);
  phase += dphase;
  return out;
}

std::array<int32_t, 2> Oscillator::iq(uint32_t phase) {
  bool q24 = phase & (1<<24);
  bool q34 = phase & (1<<25);
  int32_t val = sinetable[(phase>>16) % 256];
  if ( q24 ) {
    if ( q34 ) {
      // 24 & 34 -> 4
      return {val - 512, val};
    } else {
      // 24 & 12 -> 2
      return {512 - val, -val};
    }
  } else {
    if ( q34 ) {
      // 13 & 34 -> 3
      return {-val, val - 512};
    } else {
      // 13 & 12 -> 1
      return {val, 512 - val};
    }
  }
}
