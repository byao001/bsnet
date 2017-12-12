//
// Created by byao on 12/7/17.
// Copyright (c) 2017 byao. All rights reserved.
//

#ifndef BSNET_BYTEBUFFER_HPP
#define BSNET_BYTEBUFFER_HPP

#include "ringbuf.hpp"
#include <cstdint>

namespace bsnet {

using Byte = std::uint8_t;

class ByteBuffer {
public:
  using InnerBuf = ringbuf_t<Byte>;
  using SizeType = InnerBuf::size_type;

  static constexpr SizeType DefaultSize = 1024;

  ByteBuffer();
  explicit ByteBuffer(std::size_t cap);
  ByteBuffer(const ByteBuffer &) = default;
  ByteBuffer &operator=(const ByteBuffer &) = default;
  ByteBuffer(ByteBuffer &&other);

  Byte operator[](std::size_t index) const { return _buf[index]; }
  Byte &operator[](std::size_t index) { return _buf[index]; }

  SizeType readable_bytes() const { return _buf.readable_size(); }
  SizeType writable_bytes() const { return _buf.writable_size(); }

  void clear() { _buf.clear(); }

  void discard(SizeType n) {
    assert(n <= _buf.readable_size());
    _buf.advance_read(n);
  }

  bool starts_with(const Byte *bytes, std::size_t len) const;
  bool starts_with(const char *prefix) const;
  bool starts_with(const std::string &prefix) const {
    return starts_with(prefix.c_str());
  }
  bool ends_with(const Byte *bytes, std::size_t len) const;
  bool ends_with(const char *postfix) const;
  bool ends_with(const std::string &prefix) const {
    return ends_with(prefix.c_str());
  }

  /**
   * read bytes from a file descriptor
   */
  SizeType read_from(int fd);

  /**
   * write bytes to a file descriptor
   */
  SizeType write_to(int fd);

  /**
   * find specified byte, return 0-based index.
   */
  SizeType find(Byte b) const { return _buf.find(b); }

  // put methods
  void put(const void *data, std::size_t s);

  /**
   * put data to the buffer, without checking the writable bytes
   * user must be sure the operation is safe.
   */
  void put_fast(const void *data, std::size_t s) { _buf.append(data, s); }

  /**
   * put an arithmetic type into the buffer
   */
  template <typename T>
  std::enable_if<std::is_arithmetic<T>::value, T> put(T t) {
    put_arith(&t, sizeof(t));
  }

  /**
   * put string into the buffer
   */
  void put_string(const std::string &s) { put(s.data(), s.length()); }

  /**
   * take 's' bytes of data into 'data'
   */
  void take(void *data, std::size_t s) {
    _buf.retrieve(static_cast<Byte *>(data), static_cast<SizeType>(s));
  }

  /**
   * take a arithmetic value from the buffer
   */
  template <typename T> std::enable_if<std::is_arithmetic<T>::value, T> take() {
    assert(_buf.readable_size() >= sizeof(T));
    T v;
    take(&v, sizeof(v));
    return v;
  }

  /**
   * take all the data in the buffer out, and return as a string
   */
  std::string take_string();

  /**
   * Split the byte buffer at 'index', self became [index, len),
   * return value is [0, index)
   */
  ByteBuffer split_at(std::size_t index);

private:
  void put_arith(const void *data, std::size_t s) {
    if (_buf.writable_size() < s)
      _buf.expand();
    _buf.append(data, s);
  }

private:
  ringbuf_t<Byte> _buf;
};

} // namespace bsnet

#endif // BSNET_BYTEBUFFER_HPP