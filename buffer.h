#ifndef BUFFER_H
#define BUFFER_H
#include <mutex>

template<typename T, size_t size>
class Buffer {
  public:
    std::mutex mutex;
    Buffer() {};
    ~Buffer() {};
    inline bool full() { return tail_ == head_; }
    inline bool empty() { return (tail_ + 1) % size == head_; }
    inline size_t used() { return (head_ + (size - tail_ - 1)) % size; };
    inline size_t free() { return (tail_ + (size - head_)) % size; };
    inline size_t dropped() { return dropped_; };
    void push(T item) {
      data_[head_] = item;
      head_ = (head_ + 1) % size;
    };
    T pop() {
      tail_ = (tail_ + 1) % size;
      return data_[tail_];
    };
    void extend(T* input, size_t len) {
      size_t wlen = std::min(len, free());
      size_t len1 = std::min(wlen, size - head_);
      std::copy(input, input + len1, data_.begin() + head_);
      if ( wlen > len1 ) {
        std::copy(input + len1, input + wlen, data_.begin());
      }
      head_ = (head_ + wlen) % size;
      dropped_ += len - wlen;
    };

  private:
    std::array<T, size> data_;
    size_t head_{0};
    size_t tail_{size - 1};
    size_t dropped_{0};
};

#endif
