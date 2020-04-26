#include <chrono>
#include <iostream>
#include <thread>
#include <mutex>
#include "rtldevice.h"
#include "nco.h"
#include "buffer.h"

std::tuple<uint32_t, uint32_t> algo(RTLDevice& device, Oscillator& oscillator);

int main(void) {
  std::cout << "Device count: " << RTLDevice::count() << std::endl;
  if ( RTLDevice::count() > 0 ) {
    std::cout << "Opening device 0" << std::endl;
    RTLDevice device(0);
    constexpr uint32_t f_samp = 48000*5*5;
    constexpr double f_center = 160.5e6;
    constexpr double f_target = 160.4315e6;
    device.setFrequency((uint32_t) f_center);
    Oscillator oscillator(f_target - f_center, f_samp);
    device.setSampleRate(f_samp);
    device.setGainMode(RTLDevice::GAIN_AUTO);


    device.start();
    auto tic = std::chrono::steady_clock::now();
    uint32_t sum_mag{0};
    uint32_t n_mag{0};
    for (size_t it=0; it<5000; ++it) {
      {
        std::unique_lock<std::mutex> lock(device.output_mutex);
        device.output_cv.wait(lock, [&device]{return device.output.used() >= 2;});
        if ( it % 500 == 0 ) {
          std::cout << "Receiving: buffer @ " << device.output.used() << " bytes...";
        }
        std::tie(sum_mag, n_mag) = algo(device, oscillator);
        if ( it % 500 == 0 ) {
          float magavg = (n_mag > 0) ? sum_mag / (float) n_mag : 0.;
          std::cout << " nds: " << n_mag << ", avg mag: " << magavg << std::endl;
        }
      }
    }
    device.stop();
    auto toc = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = toc - tic;
    std::cout << "Elapsed: " << diff.count() << std::endl;
    std::cout << "Bytes read: " << device.bytesRead() << std::endl;
    std::cout << "SPS: " << device.bytesRead() / 2 / diff.count() << std::endl;
    std::cout << "Dropped: " << device.output.dropped() << std::endl;
  }
  return 0;
}
