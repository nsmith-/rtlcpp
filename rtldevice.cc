#include <thread>
#include <mutex>
#include <rtl-sdr.h>
#include "rtldevice.h"

uint32_t RTLDevice::count() {
  return rtlsdr_get_device_count();
}

RTLDevice::RTLDevice(uint32_t index) {
  status_ = rtlsdr_open(&cdev_, index);
}

RTLDevice::~RTLDevice() {
  stop();
  rtlsdr_close(cdev_);
}

void RTLDevice::setFrequency(uint32_t freq) {
  status_ = rtlsdr_set_center_freq(cdev_, freq);
}

void RTLDevice::setSampleRate(uint32_t fs) {
  status_ = rtlsdr_set_sample_rate(cdev_, fs);
}

void RTLDevice::setGainMode(Gain gain) {
  status_ = rtlsdr_set_tuner_gain_mode(cdev_, gain);
}

void RTLDevice::start() {
  if ( not rthread_ ) {
    rthread_.reset(new std::thread([this](){
        status_ = rtlsdr_reset_buffer(cdev_);
        rtlsdr_read_async(cdev_, rtl_callback_, reinterpret_cast<void*>(this), 0, 0);
    }));
  }
}

void RTLDevice::stop() {
  if ( rthread_ ) {
    rtlsdr_cancel_async(cdev_);
    rthread_->join();
    rthread_.reset(nullptr);
  }
}

uint32_t RTLDevice::bytesRead() { return bytesRead_; }

void RTLDevice::rtl_callback_(unsigned char *buf, uint32_t len, void *ctx) {
  auto *cls = reinterpret_cast<RTLDevice*>(ctx);
  cls->bytesRead_ += len;
  std::lock_guard<std::mutex> lock(cls->output.mutex);
  cls->output.extend(buf, len);
}
