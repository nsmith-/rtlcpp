#include <chrono>
#include <iostream>
#include <thread>
#include <mutex>
#include "rtldevice.h"
#include "nco.h"


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
    device.start();
    auto tic = std::chrono::steady_clock::now();
    for (size_t it=0; it<1000; ++it) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      uint32_t sum_mag{0};
      uint32_t n_mag{0};
      {
        std::lock_guard<std::mutex> lock(device.output.mutex);
        for (size_t j=0; j < device.output.used(); ++j) {
          auto iq = oscillator.pop();
          auto i = (device.output.pop() * iq[0]) / 256; // 8 * 10 -> 10
          auto q = (device.output.pop() * iq[1]) / 256; // 8 * 10 -> 10
          sum_mag += (i*i + q*q) / 1024; // 10 * 10 -> 10
        }
        n_mag += device.output.used();
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
