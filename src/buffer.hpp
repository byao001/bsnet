//
// Created by byao on 10/30/17.
// Copyright (c) 2017 byao. All rights reserved.
//

#pragma once

#include <cassert>
#include <cstdint>
#include <string>

namespace bsnet {

using byte_t = std::uint8_t;

template <typename Buf> class Buffer;

/**
 * A circular buffer, which is used in the Buffer class.
 *   Reading and writing the circular buffers should be non-exhuasting
 *   or non-exceeding.
 *   It's Buffer's responsibility to check the size of the circular
 *   buffer and resize if neccessary.
 */
class ringbuf_t {
public:
  using size_type = int;

  explicit ringbuf_t(size_type cap);
  ~ringbuf_t();

  ringbuf_t(const ringbuf_t &other);
  ringbuf_t(ringbuf_t &&other) noexcept;
  ringbuf_t &operator=(ringbuf_t other);
  ringbuf_t &operator=(ringbuf_t &&other) noexcept;

  void swap(ringbuf_t &other) noexcept;

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

  /**
   * Retrieve data from buffer.
   * @param data
                   * @param size Require size <= readable_bytes().
   * @return
   */
  size_type retrieve(void *data, size_type size);

  /**
   * Retrieve all the data from buffer.
   * @param data require sizeof(data[]) >= size()
   * @return
   */
  size_type retrieve_all(void *data);

  /**
   * Append data to buffer.
   * @param data input data
   * @param size require size < writable_bytes()
   * @return actually appended size.
   */
  size_type append(const void *data, size_type size);

  /**
   * Resize the buffer, expand the size to twice.
   * @return updated self.
   */
  ringbuf_t &expand();

  /**
   * Resize the buffer, expand the specified size.
   * If the @param size < capacity(), then do nothing.
   * @param size
   * @return
   */
  ringbuf_t &reserve(size_type size);

  friend class Buffer<ringbuf_t>;

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

void swap(ringbuf_t &lhs, ringbuf_t &rhs);

/**
 * Network Buffer, used to cache data which is sent to or received from
 * networks.
 * The network Buffer hav value semantic, which means it si copyable and
 * movable.
 *
 * @tparam TBuf inner byte buffer, the inner buffer is not required to resize
 *             automatically when space is not enough. Buffer will check the
 *             size and resize manually.
 *
 *  The inner byte buffer is required to have following interface:
 *      + constructor(size_type s);
 *      + copy and move ctor, copy and move assign operator.
 *      + void swap(But&) nonexpt;
 *      + bool empty() const;
 *      + bool full() const;
 *      + size_type size() const;
 *      + size_tyep capacity() const;
 *      + size_type readable_bytes() const;
 *      + size_type writable_bytes() const;
 *      + size_type retrieve(void *data, size_type size);
 *      + size_type retrieve_all(void *data);
 *      + size_type append(const void *data, size_type size);
 *      + Buf& expand();
 *      + Buf& reserve(size_type size);
 */
template <typename TBuf = ringbuf_t> class Buffer {
public:
  using size_type = typename TBuf::size_type;
  static const std::size_t DefaultInitSize = 1024;

  explicit Buffer(int len = DefaultInitSize) : _buf(len) {}
  // implicitly generated copy and move ctor, and dtor

  void swap(Buffer<TBuf> &other) noexcept { _buf.swap(other._buf); }

  ///
  /// Properties fo the buffer, just similar to the inner buffer,
  /// which in default case, is ringbuf_t.
  bool empty() const { return _buf.empty(); }
  bool full() const { return _buf.full(); }
  size_type size() const { return _buf.size(); }
  size_type capacity() const { return _buf.capacity(); }

  /**
   * Forward to inner buffer's retrieve method.
   * @param data
   * @param size
   * @return
   */
  size_type retrieve(void *data, size_type size) {
    return _buf.retrieve(data, size);
  }

  /**
   * Forward to inner buffer's retrieve_all method.
   * @param data
   * @return
   */
  size_type retrieve_all(void *data) { return _buf.retrieve_all(data); }

  /**
   * Retrieve bytes and return the bytes as a string.
   * @param size
   * @return
   */
  std::string retrieve_string(size_type size) {
    char bf[1024];
    std::string s;
    while (size > 1024) {
      retrieve(&bf[0], 1024);
      s.append(&bf[0], 1024);
      size -= 1024;
    }
    retrieve(&bf[0], size);
    s.append(&bf[0], size);
    return s;
  }

  /**
   * Append specified size of data to the inner buffer. When inner buffer
   * doesn't have enough space,
   * expand the inner buffer automatically.
   * @param data
   * @param size
   */
  const void append(const void *data, size_type size) {
    if (size > _buf.writable_bytes()) {
      if (_buf.expand().writable_bytes() < size)
        _buf.reserve(_buf.size() + size);
    }
    _buf.append(data, size);
  }

  /**
   * Append a string as bytes to the inner buffer.
   * @param str
   */
  const void append(const std::string &str) { append(str.c_str(), str.size()); }

private:
  TBuf _buf;
};

template <typename TBuf> void swap(Buffer<TBuf> &lhs, Buffer<TBuf> &rhs) {
  lhs.swap(rhs);
}
}
