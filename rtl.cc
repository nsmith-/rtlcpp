#include <chrono>
#include <iostream>
#include <thread>
#include <mutex>
#include "rtldevice.h"
#include "nco.h"
#include "buffer.h"


const std::array<int32_t, 31> downsample4_fir {
    2,   4,   6,   6,   1,  -7, -17, -26, -26, -14,  13,  52, 101,
  144, 177, 194, 177, 144, 101,  52,  13, -14, -26, -26, -17,  -7,
    1,   6,   6,   4,   2
};

int main(void) {
  std::cout << "Device count: " << RTLDevice::count() << std::endl;
  if ( RTLDevice::count() > 0 ) {
    std::cout << "Opening device 0" << std::endl;
    RTLDevice device(0);
    uint32_t fsamp = 2560000;
    device.setFrequency(91100000);
    Oscillator oscillator(2.0e5, fsamp);
    device.setSampleRate(fsamp);
    device.setGainMode(RTLDevice::GAIN_AUTO);

    Buffer<int32_t, 1024> downconverted_i;
    Buffer<int32_t, 1024> downconverted_q;
    Buffer<int32_t, 1024> downsample1_i;
    Buffer<int32_t, 1024> downsample1_q;

    device.start();
    auto tic = std::chrono::steady_clock::now();
    for (size_t it=0; it<500; ++it) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      uint32_t sum_mag{0};
      uint32_t n_mag{0};
      {
        std::lock_guard<std::mutex> lock(device.output.mutex);
        while ( device.output.used() >= 2 ) {
          auto iq = oscillator.pop();
          // 7 * 10 -> 10
          downconverted_i.push((device.output.pop() * iq[0]) / 128);
          downconverted_q.push((device.output.pop() * iq[1]) / 128);

          if ( downconverted_i.used() > downsample4_fir.size() ) {
            uint32_t firsum_i{0};
            uint32_t firsum_q{0};
            auto ivec = downconverted_i.slice(downsample4_fir.size());
            auto qvec = downconverted_i.slice(downsample4_fir.size());
            for (size_t i=0; i < downsample4_fir.size(); ++i) {
              firsum_i += downsample4_fir[i] * ivec[i];
              firsum_q += downsample4_fir[i] * qvec[i];
            }
            downsample1_i.push(firsum_i / 1024);
            downsample1_q.push(firsum_q / 1024);
            downconverted_i.advance(4);
            downconverted_q.advance(4);
          }

          if ( not downsample1_i.empty() ) {
            auto i = downsample1_i.pop();
            auto q = downsample1_q.pop();
            sum_mag += (i*i + q*q) / 1024;
            n_mag++;
          }
        }
      }
      float magavg = (n_mag > 0) ? sum_mag / (float) n_mag : 0.;
      if ( it % 100 == 0 ) std::cout << "Receiving: buffer @ " << device.output.used() << " bytes, avg mag: " << magavg << std::endl;
    }
    device.stop();
    auto toc = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = toc - tic;
    std::cout << "Elapsed: " << diff.count() << std::endl;
    std::cout << "Bytes read: " << device.bytesRead() << std::endl;
    std::cout << "Dropped: " << device.output.dropped() << std::endl;
  }
  return 0;
}
