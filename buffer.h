#ifndef BUFFER_H
#define BUFFER_H
#include <array>

template<typename T, size_t size>
class Buffer {
  public:
    Buffer() {};
    ~Buffer() {};
    inline bool full() const { return tail_ == head_; }
    inline bool empty() const { return (tail_ + 1) % size == head_; }
    inline size_t used() const { return (head_ + (size - tail_ - 1)) % size; };
    inline size_t free() const { return (tail_ + (size - head_)) % size; };
    inline size_t dropped() const { return dropped_; };
    inline T get(size_t idx) const { return data_[(tail_ + 1 + idx) % size]; };
    inline void advance(size_t n=1) { tail_ = (tail_ + n) % size; };
    inline void push(T item) {
      data_[head_] = item;
      head_ = (head_ + 1) % size;
    };
    inline T pop() {
      advance();
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
