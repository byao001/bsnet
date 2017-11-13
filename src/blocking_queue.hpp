//
// Created by byao on 11/8/17.
// Copyright (c) 2017 byao. All rights reserved.
//

#pragma once

#include "NonCopyable.hpp"
#include <chrono>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <utility>
#include <vector>

namespace bsnet {

/**
 * A blocking queue with unlimited size.
 * @tparam T
 */
template<typename T> class blocking_queue_t : public NonCopyable {
public:
  void get(T &t) {
    std::unique_lock<std::mutex> lk(_mtx);

    while (_queue.empty())
      _no_empty.wait(lk);

    t = std::move(_queue.front());
    _queue.pop_front();
  }

  void get_all(std::vector<T> &res) {
    std::unique_lock<std::mutex> lk(_mtx);

    while (_queue.empty())
      _no_empty.wait(lk);

    std::copy(_queue.begin(), _queue.end(), std::back_inserter(res));
    _queue.clear();
  }

  void put(const T &t) {
    std::unique_lock<std::mutex> lk(_mtx);
    _queue.push_back(t);
    _no_empty.notify_one();
  }
  void put(T &&t) {
    std::unique_lock<std::mutex> lk(_mtx);
    _queue.push_back(std::forward<T>(t));
    _no_empty.notify_one();
  }

  template<typename... Args> void emplace(Args &&... args) {
    std::unique_lock<std::mutex> lk(_mtx);
    _queue.emplace_back(std::forward<Args>(args)...);
    _no_empty.notify_one();
  }

  void put_all(std::vector<T> &vec) {
    std::unique_lock<std::mutex> lk(_mtx);
    std::copy(vec.begin(), vec.end(), std::back_inserter(_queue));
    _no_empty.notify_one();
  }

  bool empty() const {
    std::unique_lock<std::mutex> lk(_mtx);
    return _queue.empty();
  }

  std::size_t size() const {
    std::unique_lock<std::mutex> lk(_mtx);
    return _queue.size();
  }

private:
  std::mutex _mtx;
  std::condition_variable _no_empty;
  std::deque<T> _queue;
};

/**
 * A blocking queue with fixed size.
 * @tparam T
 */
template<typename T> class bounded_blocking_queue_t : NonCopyable {
public:
  explicit bounded_blocking_queue_t(std::size_t s) : _max_size(s) {}

  void get(T &t) {
    std::unique_lock<std::mutex> lk(_mtx);

    while (_queue.empty())
      _no_empty.wait(lk);

    t = std::move(_queue.front());
    _queue.pop_front();
    _no_full.notify_one();
  }

  void get_all(std::vector<T> &res) {
    std::unique_lock<std::mutex> lk(_mtx);

    while (_queue.empty())
      _no_empty.wait(lk);

    std::copy(_queue.begin(), _queue.end(), std::back_inserter(res));
    _queue.clear();
    _no_full.notify_one();
  }

  void put(T &t) {
    std::unique_lock<std::mutex> lk(_mtx);
    while (_queue.size() >= _max_size)
      _no_full.wait(lk);

    _queue.push_back(t);
    _no_empty.notify_one();
  }

  void put(T &&t) {
    std::unique_lock<std::mutex> lk(_mtx);
    while (_queue.size() >= _max_size)
      _no_full.wait(lk);

    _queue.push_back(std::forward<T>(t));
    _no_empty.notify_one();
  }

  std::size_t put_all(std::vector<T> &vec) {
    std::unique_lock<std::mutex> lk(_mtx);

    while (_queue.size() >= _max_size)
      _no_full.wait(lk);
    std::size_t n = _max_size - _queue.size();
    std::copy_n(vec.begin(), n, std::back_inserter(_queue));
    _no_empty.notify_one();

    return vec.size() - n;
  }

  template<typename... Args> void emplace(Args &&... args) {
    std::unique_lock<std::mutex> lk(_mtx);
    while (_queue.size() >= _max_size)
      _no_full.wait(lk);

    _queue.emplace_back(std::forward<Args>(args)...);
    _no_empty.notify_one();
  }

  bool empty() const {
    std::unique_lock<std::mutex> lk(_mtx);
    return _queue.empty();
  }

  bool full() const {
    std::unique_lock<std::mutex> lk(_mtx);
    return _queue.size() >= _max_size;
  }
  std::size_t size() const {
    std::unique_lock<std::mutex> lk(_mtx);
    return _queue.size();
  }

private:
  std::mutex _mtx;
  std::condition_variable _no_empty;
  std::condition_variable _no_full;
  std::deque<T> _queue;
  std::size_t _max_size;
};
}
