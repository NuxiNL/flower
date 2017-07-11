// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#ifndef FLOWER_UTIL_LOGGER_H
#define FLOWER_UTIL_LOGGER_H

#include <iomanip>
#include <mutex>
#include <ostream>
#include <thread>

namespace flower {
namespace util {
namespace {

// Log entry that is in the process of being written. This class
// serializes output to the stream and also ensures all entries have a
// trailing newline.
class LogTransaction {
 public:
  LogTransaction(std::ostream* ostream, std::mutex* ostream_lock)
      : ostream_(ostream), ostream_guard_(*ostream_lock) {
  }

  ~LogTransaction() {
    *ostream_ << std::endl;
  }

  template <typename T>
  LogTransaction& operator<<(const T& v) {
    *ostream_ << v;
    return *this;
  }

 private:
  std::lock_guard<std::mutex> ostream_guard_;
  std::ostream* ostream_;
};

// Wrapper around std::ostream for writing log entries.
//
// C++ streams are not required to be thread-safe. This class protects
// an output stream with a mutex.
class Logger {
 public:
  explicit Logger(std::ostream* ostream) : ostream_(ostream) {
  }

  // TODO(ed): Should we add support for logging severities?
  LogTransaction Log() {
    return LogTransaction(ostream_, &ostream_lock_);
  }

 private:
  std::mutex ostream_lock_;
  std::ostream* ostream_;
};

}  // namespace
}  // namespace util
}  // namespace flower

#endif
