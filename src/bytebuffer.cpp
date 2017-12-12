//
// Created by byao on 12/7/17.
// Copyright (c) 2017 byao. All rights reserved.
//

#include "bytebuffer.hpp"
#include "ringbuf.hpp"
#include <cstdint>
#include <string>

using std::size_t;
using std::string;

namespace bsnet {

using SizeType = ByteBuffer::SizeType;

ByteBuffer::ByteBuffer(size_t cap) : _buf(cap) {}

ByteBuffer::ByteBuffer() : ByteBuffer(ByteBuffer::DefaultSize) {}

ByteBuffer::ByteBuffer(ByteBuffer &&other) : _buf(std::move(other._buf)) {}

void ByteBuffer::put(const void *data, size_t s) {
  if (s > _buf.writable_size()) {
    if (_buf.expand().writable_size() < s)
      _buf.reserve(_buf.readable_size() + s);
  }
  _buf.append(data, static_cast<SizeType>(s));
}

string ByteBuffer::take_string() {
  std::string s;
  SizeType rsize = _buf.readable_size();

  SizeType part = std::min(rsize, _buf._capacity - _buf._begin);
  s.append(reinterpret_cast<char *>(_buf._data + _buf._begin), part);

  if (part < rsize) {
    s.append(reinterpret_cast<char *>(_buf._data), _buf._end);
  }
  _buf.clear();
  return s;
}

ByteBuffer ByteBuffer::split_at(size_t index) {
  assert(index < _buf.readable_size());
  ByteBuffer buffer(index);
  this->take(buffer._buf._data, index);
  buffer._buf.advance_write(index);
  return buffer;
}

SizeType ByteBuffer::read_from(int fd) {
  if (_buf.writable_size() == 0)
    _buf.expand();

  static constexpr SizeType BufSize = 65536 >> 1;
  Byte extrabuf[BufSize];
  struct iovec vio[3];
  int len = 2;
  if (_buf._end < _buf._begin) {
    vio[0].iov_base = _buf._data + _buf._end;
    vio[0].iov_len = _buf._begin - _buf._end - 1;
    vio[1].iov_base = &extrabuf[0];
    vio[1].iov_len = BufSize;
  } else {
    vio[0].iov_base = _buf._data + _buf._end;
    vio[0].iov_len = _buf._capacity - _buf._end;
    if (_buf._begin > 1) {
      vio[1].iov_base = _buf._data;
      vio[1].iov_len = _buf._begin - 1;
      len = 3;
    }
    vio[len - 1].iov_base = &extrabuf[0];
    vio[len - 1].iov_len = BufSize;
  }
  int n = ::readv(fd, &vio[0], len);
  if (n == -1) {
    // TODO: replace perror with a logger
    perror("readv in \"ByteBuffer::read_from\"");
    return n;
  }
  int rest = n - _buf.writable_size();
  if (rest > 0) {
    _buf.advance_write(_buf.writable_size());
    _buf.append(&extrabuf[0], rest);
  } else {
    _buf.advance_write(n);
  }
  return n;
}

SizeType ByteBuffer::write_to(int fd) {
  if (_buf.readable_size() == 0)
    return 0;
  struct iovec vio[2];
  int len;
  if (_buf._end < _buf._begin) {
    vio[0].iov_base = _buf._data + _buf._begin;
    vio[0].iov_len = _buf._capacity - _buf._begin;
    vio[1].iov_base = _buf._data;
    vio[1].iov_len = _buf._end;
    len = 2;
  } else {
    vio[0].iov_base = _buf._data + _buf._begin;
    vio[0].iov_len = _buf._end - _buf._begin;
    len = 1;
  }
  int n = ::writev(fd, &vio[0], len);
  if (n != -1)
    _buf.advance_read(n);
  else // TODO: replace perror with a logger
    perror("writev in \"ByteBuffer::write_to\"");
  return n;
}

bool ByteBuffer::starts_with(const Byte *data, size_t len) const {
  for (int i = 0; i < len; ++i) {
    if (data[i] != _buf[i])
      return false;
  }
  return true;
}

bool ByteBuffer::starts_with(const char *prefix) const {
  for (int i = 0; prefix[i] != '\0'; ++i) {
    if (prefix[i] != _buf[i])
      return false;
  }
  return true;
}

bool ByteBuffer::ends_with(const Byte *data, size_t len) const {
  int buf_len = _buf.readable_size();
  for (int i = 1; i <= len; ++i) {
    if (data[len - i] != _buf[buf_len - i])
      return false;
  }
  return true;
}

bool ByteBuffer::ends_with(const char *prefix) const {
  int buf_len = _buf.readable_size();
  int pre_len = ::strlen(prefix);
  for (int i = 1; i <= pre_len; ++i) {
    if (prefix[pre_len - i] != _buf[buf_len - i])
      return false;
  }
  return true;
}

} // namespace bsnet
