#include <iostream>
#include <math.h>
#include "nco.h"

bool Oscillator::sinetable_init = false;
std::array<int32_t, Oscillator::tablesize> Oscillator::sinetable;

Oscillator::Oscillator(double frequency, uint32_t sample_rate, double initial_phase) {
  if ( not sinetable_init ) {
    for (size_t i=0; i<tablesize; ++i) {
      // 10 bit table (4 quadrants done later), 10 fractional bits output
      sinetable[i] = std::round(1024 * std::sin(i * (2*M_PI / tablesize / 4)));
    }
    sinetable_init = true;
  }
  if ( std::abs(frequency) >= sample_rate / 2 ) {
    std::cerr << "Warning: NCO with f outside +/- f_samp/2" << std::endl;
  }
  if ( frequency < 0. ) {
    frequency = sample_rate - frequency;
  }
  // 16 bits of dithering
  phase = std::round((65536 * 1024 / 2 / M_PI) * initial_phase);
  dphase = std::round(65536 * 1024 * frequency / sample_rate);
}

std::array<int32_t, 2> Oscillator::pop() {
  bool q24 = phase & (1<<(16 + 8));
  bool q34 = phase & (1<<(16 + 8 + 1));
  int32_t val = sinetable[(phase>>16) % tablesize];
  phase += dphase;
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
