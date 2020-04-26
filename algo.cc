#include <array>
#include "rtldevice.h"
#include "buffer.h"
#include "nco.h"

// const std::array<int32_t, 31> downsample4_fir {
//     2,   4,   6,   6,   1,  -7, -17, -26, -26, -14,  13,  52, 101,
//   144, 177, 194, 177, 144, 101,  52,  13, -14, -26, -26, -17,  -7,
//     1,   6,   6,   4,   2
// };
constexpr std::array<int32_t, 15> downsample4_fir {
  -8,  -4,   8,  36,  84, 132, 172, 192, 172, 132,  84,  36,   8, -4,  -8
};
Buffer<int32_t, downsample4_fir.size() + 1> downconverted_i;
Buffer<int32_t, downsample4_fir.size() + 1> downconverted_q;
Buffer<int32_t, 16> downsample1_i;
Buffer<int32_t, 16> downsample1_q;
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
      for (size_t i=0; i < downsample4_fir.size(); ++i) {
        firsum_i += downsample4_fir[i] * downconverted_i.get(i);
        firsum_q += downsample4_fir[i] * downconverted_q.get(i);
      }
      downsample1_i.push(firsum_i / 1024);
      downsample1_q.push(firsum_q / 1024);
      downconverted_i.advance(4);
      downconverted_q.advance(4);

      if ( not downsample1_i.empty() ) {
        auto i = downsample1_i.pop();
        auto q = downsample1_q.pop();
        sum_mag += (i*i + q*q) / 1024;
        n_mag++;
      }
    }
  }
  return {sum_mag, n_mag};
}
