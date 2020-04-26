#ifndef RTLDEVICE_H
#define RTLDEVICE_H
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "buffer.h"

struct rtlsdr_dev;

class RTLDevice {
  public:
    enum Gain : int { GAIN_AUTO=0, GAIN_MANUAL=1 };
    typedef Buffer<unsigned char, 2*16*16384> Buffer_t;
    Buffer_t output;
    std::mutex output_mutex;
    std::condition_variable output_cv;

    static uint32_t count();
    RTLDevice(uint32_t index);
    ~RTLDevice();
    void setFrequency(uint32_t freq);
    void setSampleRate(uint32_t fs);
    void setGainMode(Gain gain);
    void start();
    void stop();
    uint32_t bytesRead();

  private:
    rtlsdr_dev * cdev_;
    std::atomic<int> status_{0};
    std::unique_ptr<std::thread> rthread_{nullptr};
    uint32_t bytesRead_{0};

    static void rtl_callback_(unsigned char *buf, uint32_t len, void *ctx);
};

#endif
