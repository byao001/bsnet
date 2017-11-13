//
// Created by byao on 10/30/17.
// Copyright (c) 2017 byao. All rights reserved.
//
#include "buffer.hpp"
#include <cstring>

namespace bsnet {

using size_type = ringbuf_t::size_type;

void swap(ringbuf_t &lhs, ringbuf_t &rhs) {
  puts("swap in bsnet\n");
  lhs.swap(rhs);
}

ringbuf_t::ringbuf_t(size_type cap)
    : _data(new std::uint8_t[cap]), _capacity(cap), _size(0), _begin(0) {}

ringbuf_t::~ringbuf_t() { delete[] _data; }

ringbuf_t::ringbuf_t(const ringbuf_t &other)
    : _data(new std::uint8_t[other._capacity]), _capacity(other._capacity),
      _size(other._size), _begin(other._begin) {}

ringbuf_t::ringbuf_t(ringbuf_t &&other) noexcept : _data(nullptr) { // NOLINT
  swap(other);
}

ringbuf_t &ringbuf_t::operator=(ringbuf_t other) {
  swap(other);
  return *this;
}

ringbuf_t &ringbuf_t::operator=(ringbuf_t &&other) noexcept {
  swap(other);
  return *this;
}

void ringbuf_t::swap(ringbuf_t &other) noexcept {
  using std::swap;
  swap(_data, other._data);
  swap(_capacity, other._capacity);
  swap(_size, other._size);
  swap(_begin, other._begin);
}

size_type ringbuf_t::retrieve(void *data, size_type size) {
  size_type bytes = std::min(size, readable_bytes());
  size_type part = std::min(bytes, _capacity - _begin);

  memcpy(data, _data + _begin, static_cast<size_t>(part));
  if (bytes > part) {
    memcpy(static_cast<byte_t *>(data) + part, _data,
           static_cast<size_t>(bytes - part));
  }
  // update size and _begin index.
  read(bytes);
  return bytes;
}

size_type ringbuf_t::retrieve_all(void *data) { return retrieve(data, size()); }

size_type ringbuf_t::append(const void *data, size_type size) {
  size_type bytes = std::min(size, writable_bytes());
  size_type tail = (_begin + _size) % _capacity;
  size_type part = std::min(bytes, _capacity - tail);

  memcpy(_data + tail, data, static_cast<size_t>(part));
  if (bytes > part) {
    memcpy(_data, static_cast<const byte_t *>(data) + part,
           static_cast<size_t>(bytes - part));
  }
  // update size and _end index.
  write(bytes);
  return bytes;
}

ringbuf_t &ringbuf_t::expand() { return reserve(_capacity << 1); }

ringbuf_t &ringbuf_t::reserve(size_type size) {
  if (size <= _capacity)
    return *this;

  byte_t *new_data = new byte_t[size];

  size_type part = std::min(_size, _capacity - _begin);
  memcpy(new_data, _data + _begin, static_cast<size_t>(part));
  if (_size > part) {
    memcpy(new_data + part, _data, static_cast<size_t>(_size - part));
  }

  _capacity = size;
  _begin = 0;
  delete[] _data;
  _data = new_data;

  return *this;
}
}
