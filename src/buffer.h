//
// Created by pandabo on 10/30/17.
//

#pragma once

#include <cassert>
#include <cstdint>
#include <string>

namespace bsnet {

using byte_t = std::uint8_t;

/*
   * A circular buffer, which is used in the Buffer class.
   *   Reading and writing the circular buffers should be non-exhuasting
   * or non-exceeding.
   *   It's Buffer's responsibility to check the size of the circular
   * buffer and resize if neccessary.
   */
class ringbuf_t {
public:
  using size_type = int;

  explicit ringbuf_t(size_type cap)
      : _data(new std::uint8_t[cap]), _capacity(cap), _size(0), _begin(0) {}

  ~ringbuf_t() { delete[] _data; }

  ringbuf_t(const ringbuf_t &other)
      : _data(new std::uint8_t[other._capacity]), _capacity(other._capacity),
        _size(other._size), _begin(other._begin) {}

  ringbuf_t(ringbuf_t &&other) noexcept : _data(nullptr) {
    std::swap(_data, other._data);
    std::swap(_capacity, other._capacity);
    std::swap(_size, other._size);
    std::swap(_begin, other._begin);
  }

  ringbuf_t &operator=(ringbuf_t other) {
    swap(other);
    return *this;
  }

  ringbuf_t &operator=(ringbuf_t &&other) noexcept {
    swap(other);
    return *this;
  }

  void swap(ringbuf_t &other) {
    std::swap(_data, other._data);
    std::swap(_capacity, other._capacity);
    std::swap(_size, other._size);
    std::swap(_begin, other._begin);
  }

  bool empty() const { return _size == 0; }
  bool full() const { return _size == _capacity; }
  size_type capacity() const { return _capacity; }
  size_type size() const { return _size; }

  /*
   * Readable bytes starts from _begin, ends up to _end
   */
  size_type readable_bytes() const { return _size; }

  /*
   * Writable bytes starts from _end, ends up to _begin
   */
  size_type writable_bytes() const { return _capacity - _size; }

  /*
   * Retrieve data from buffer.
   *
   * Require size to be smaller than readable bytes.
   */
  size_type retrieve(void *data, size_type size) {
    size_type bytes = std::min(size, readable_bytes());
    size_type part = std::min(bytes, _capacity - _begin);
    memcpy(data, _data + _begin, part);
    if (bytes > part) {
      memcpy(static_cast<byte_t *>(data) + part, _data, bytes - part);
    }
    // update size and _begin index.
    read(bytes);
    return bytes;
  }

  /*
   * Append data to buffer.
   *
   * Require size to be smaller than writable bytes.
   */
  size_type append(const void *data, size_type size) {
    size_type bytes = std::min(size, writable_bytes());
    size_type tail = (_begin + _size) % _capacity;
    size_type part = std::min(bytes, _capacity - tail);
    memcpy(_data + tail, data, part);
    if (bytes > part) {
      memcpy(_data, static_cast<const byte_t*>(data) + part, bytes - part);
    }
    // update size and _end index.
    write(bytes);
    return bytes;
  }

  /*
   * Resize the buffer, expand the size to twice
   */
  void expand() {
    using value_type = std::uint8_t;
    byte_t *new_data = new byte_t[_capacity * 2];

    size_type part = std::min(_size, _capacity - _begin);
    memcpy(new_data, _data + _begin, part);
    if (_size > part) {
      memcpy(new_data + part, _data, _size - part);
    }
    _capacity *= 2;
    _begin = 0;
    delete[] _data;
    _data = new_data;
  }

  friend class Buffer;

private:
  /*
   * Update the begin index and size, after retrieve data of 'size'
   */
  void read(size_type size) {
    _begin = (_begin + size) % _capacity;
    _size -= size;
  }

  /*
   * Update the size, after append data of 'size'
   */
  void write(size_type size) { _size += size; }

  byte_t *_data;
  size_type _capacity;
  size_type _size;
  int _begin;
};

class Buffer {
public:
  static const std::size_t DefaultInitSize = 1024;

  Buffer(int len);
  ~Buffer();

  Buffer(const Buffer &other);
  Buffer(Buffer &&other) noexcept;
  Buffer &operator=(const Buffer &other);
  Buffer &operator=(Buffer &&other) noexcept;

  void swap(Buffer &other);
  void retrieve(int size);
  void retrieve_all();
  const byte_t *peek();
  const void append(const void *data, std::size_t size);
  const void append(const std::string &str);

  int readable_bytes() const;
  int writable_bytes() const;

private:
};
}

