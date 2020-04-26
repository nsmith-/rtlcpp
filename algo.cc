#include <array>
#include "rtldevice.h"
#include "buffer.h"
#include "nco.h"

constexpr std::array<int32_t, 31> lowpass_fir_taps {
  3,   4,   5,   3,  -1,  -7, -12, -15, -13,  -4,  10,  30,  52, 71,  85,  93,  85,  71,  52,  30,  10,  -4, -13, -15, -12,  -7, -1,   3,   5,   4,   3
};
constexpr size_t lowpass_fir_div{5};
constexpr size_t lowpass_fir_frac{512};
Buffer<int32_t, lowpass_fir_taps.size() + 1> downconverted_i;
Buffer<int32_t, lowpass_fir_taps.size() + 1> downconverted_q;
Buffer<int32_t, lowpass_fir_taps.size() + 1> downsample1_i;
Buffer<int32_t, lowpass_fir_taps.size() + 1> downsample1_q;
Buffer<int32_t, lowpass_fir_taps.size() + 1> downsample2_i;
Buffer<int32_t, lowpass_fir_taps.size() + 1> downsample2_q;
uint32_t sum_mag{0};
uint32_t n_mag{0};

std::tuple<uint32_t, uint32_t> algo(RTLDevice& device, Oscillator& oscillator) {
  sum_mag = 0;
  n_mag = 0;
  size_t nqueue = std::min(16384u, device.output.used() / 2);
  while ( nqueue-- > 0 ) {
    auto iq = oscillator.pop();
    // 7 * 10 -> 10
    downconverted_i.push((device.output.pop() * iq[0]) / 128);
    downconverted_q.push((device.output.pop() * iq[1]) / 128);

    if ( downconverted_i.full() ) {
      uint32_t firsum_i{0};
      uint32_t firsum_q{0};
      for (size_t i=0; i < lowpass_fir_taps.size(); ++i) {
        firsum_i += lowpass_fir_taps[i] * downconverted_i.get(i);
        firsum_q += lowpass_fir_taps[i] * downconverted_q.get(i);
      }
      downsample1_i.push(firsum_i / lowpass_fir_frac);
      downsample1_q.push(firsum_q / lowpass_fir_frac);
      downconverted_i.advance(lowpass_fir_div);
      downconverted_q.advance(lowpass_fir_div);

      if ( downsample1_i.full() ) {
        uint32_t firsum_i{0};
        uint32_t firsum_q{0};
        for (size_t i=0; i < lowpass_fir_taps.size(); ++i) {
          firsum_i += lowpass_fir_taps[i] * downsample1_i.get(i);
          firsum_q += lowpass_fir_taps[i] * downsample1_q.get(i);
        }
        downsample2_i.push(firsum_i / lowpass_fir_frac);
        downsample2_q.push(firsum_q / lowpass_fir_frac);
        downsample1_i.advance(lowpass_fir_div);
        downsample1_q.advance(lowpass_fir_div);

        if ( not downsample2_i.empty() ) {
          auto i = downsample2_i.pop();
          auto q = downsample2_q.pop();
          sum_mag += (i*i + q*q) / 1024;
          n_mag++;
        }
      }
    }
  }
  return {sum_mag, n_mag};
}
