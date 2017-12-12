//
// Created by byao on 10/30/17.
// Copyright (c) 2017 byao. All rights reserved.
//

#ifndef BSNET_BUFFER_HPP
#define BSNET_BUFFER_HPP

#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <sys/uio.h>
#include <typeinfo>
#include <unistd.h>

namespace bsnet {

/**
 * Network Buffer, used to cache data which is sent to or received from
 * networks.
 * The network Buffer has value semantic, which means it is copyable and
 * movable.
 */
template <typename T> class ringbuf_t {
public:
  using size_type = int;
  using self_type = ringbuf_t<T>;
  using value_type = T;

  friend class TcpStream;
  friend class ByteBuffer;

  explicit ringbuf_t(std::size_t cap)
      : _data(new value_type[cap + 1]), _capacity(cap + 1), _begin(0), _end(0) {
  }

  ~ringbuf_t() { delete[] _data; }

  ringbuf_t(const ringbuf_t &other)
      : _data(new value_type[other._capacity]), _capacity(other._capacity),
        _begin(0), _end(other.readable_size()) {
    auto bytes = other._end > other._begin ? other._end - other._begin
                                           : other._capacity - other._begin;
    memcpy(_data, other._data + other._begin, sizeof(value_type) * bytes);
    if (_end < _begin) {
      memcpy(_data + bytes, other._data, sizeof(value_type) * _end);
    }
  }

  ringbuf_t(ringbuf_t &&other) noexcept : _data(nullptr) { // NOLINT
    swap(other);
  }
  ringbuf_t &operator=(ringbuf_t other) {
    swap(other);
    return *this;
  }
  ringbuf_t &operator=(ringbuf_t &&other) noexcept {
    swap(other);
    return *this;
  }

  void swap(ringbuf_t &other) noexcept {
    using std::swap;
    swap(_data, other._data);
    swap(_capacity, other._capacity);
    swap(_end, other._end);
    swap(_begin, other._begin);
  }

  bool empty() const { return _begin == _end; }
  bool full() const { return _end == _begin - 1; }
  size_type capacity() const { return _capacity - 1; }

  /**
   * Readable bytes starts from _begin, ends up to _end
   */
  size_type readable_size() const {
    return _end >= _begin ? _end - _begin : (_capacity - _begin + _end);
  }

  /**
   * Writable bytes starts from _end, ends up to _begin
   */
  size_type writable_size() const {
    return _end < _begin ? _begin - _end - 1 : _capacity - _end + _begin - 1;
  }

  /**
   * find specified value
   */
  size_type find(T &t) const {
    if (_end > _begin) {
      for (int i = _begin; i < _end; ++i) {
        if (t == _data[i])
          return i - _begin;
      }
    } else {
      for (int i = _begin; i < _capacity; ++i) {
        if (t == _data[i])
          return i - _begin;
      }
      for (int i = 0; i < _end; ++i) {
        if (t == _data[i])
          return _capacity - _begin + i;
      }
    }
    return -1;
  }

  /**
   * Retrieve data from buffer.
   * @param data
   * @param size Require size <= readable_bytes().
   * @return
   */
  size_type retrieve(value_type *data, size_type size) {
    size_type bytes = std::min(size, readable_size());
    size_type part = std::min(bytes, _capacity - _begin);

    memcpy(data, _data + _begin,
           static_cast<size_t>(part) * sizeof(value_type));
    if (bytes > part) {
      memcpy(static_cast<value_type *>(data) + part, _data,
             static_cast<size_t>(bytes - part) * sizeof(value_type));
    }
    // update size and _begin index.
    advance_read(bytes);
    return bytes;
  }

  /**
   * Retrieve all the data from buffer.
   * @param data require sizeof(data[]) >= size()
   * @return
   */
  size_type retrieve_all(void *data) { return retrieve(data, readable_size()); }

  /**
   * Append data to buffer.
   * @param data input data
   * @param size require size < writable_bytes()
   * @return actually appended size.
   */
  size_type append(const void *data, size_type size) {
    size_type bytes = std::min(size, writable_size());
    size_type part = std::min(bytes, _capacity - _end);

    memcpy(_data + _end, data, static_cast<size_t>(part));
    if (bytes > part) {
      memcpy(_data, static_cast<const value_type *>(data) + part,
             static_cast<size_t>(bytes - part) * sizeof(value_type));
    }
    // update size and _end index.
    advance_write(bytes);
    return bytes;
  }

  /**
   * Resize the buffer, expand the size to twice.
   * @return updated self.
   */
  self_type &expand() { return reserve(_capacity << 1); }

  /**
   * Resize the buffer, expand the specified size.
   * If the @param size < capacity(), then do nothing.
   * @param size
   * @return
   */
  self_type &reserve(size_type size) {
    if (size <= capacity())
      return *this;

    value_type *new_data = new value_type[size + 1];
    auto rsize = readable_size();
    size_type part = std::min(rsize, _capacity - _begin);
    memcpy(new_data, _data + _begin,
           static_cast<size_t>(part) * sizeof(value_type));
    if (rsize > part) {
      memcpy(new_data + part, _data,
             static_cast<size_t>(rsize - part) * sizeof(value_type));
    }

    _capacity = size + 1;
    _begin = 0;
    _end = rsize;
    delete[] _data;
    _data = new_data;

    return *this;
  }

  /**
   * Clear the buffer
   */
  void clear() {
    _begin = 0;
    _end = 0;
  }

  value_type &operator[](std::size_t index) {
    assert(index < _capacity);
    return _data[(index + _begin) % _capacity];
  }

  value_type operator[](std::size_t index) const {
    assert(index < _capacity);
    return _data[(index + _begin) % _capacity];
  }

private:
  /*
   * Update the begin index and size, after retrieve data of 'size'
   */
  void advance_read(size_type size) {
    assert(readable_size() >= size);
    _begin = (_begin + size) % _capacity;
  }

  /*
   * Update the size, after append data of 'size'
   */
  void advance_write(size_type size) {
    assert(writable_size() >= size);
    _end = (_end + size) % _capacity;
  }

  // data member
private:
  value_type *_data;
  size_type _capacity;
  int _begin, _end;
};

template <typename T> void swap(ringbuf_t<T> &lhs, ringbuf_t<T> &rhs) noexcept {
  lhs.swap(rhs);
}

} // namespace bsnet

#endif // !BSNET_BUFFER_HPP
