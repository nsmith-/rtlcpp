#ifndef NCO_H
#define NCO_H
#include <array>

class Oscillator {
  public:
    Oscillator(double frequency, uint32_t sample_rate, double initial_phase=0.);
    std::array<int32_t, 2> pop();

  private:
    static std::array<int32_t, 256> sinetable;
    static bool sinetable_init;
    uint32_t phase{0};
    uint32_t dphase;
};

#endif
