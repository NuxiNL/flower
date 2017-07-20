// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#ifndef FLOWER_UTIL_LOGGER_H
#define FLOWER_UTIL_LOGGER_H

#include <iomanip>
#include <mutex>
#include <ostream>
#include <streambuf>
#include <thread>

namespace flower {
namespace util {
namespace {

// Log entry that is in the process of being written. This class
// serializes output to the stream and also ensures all entries have a
// trailing newline.
class LogTransaction : public std::ostream {
 public:
  LogTransaction(std::streambuf* streambuf, std::mutex* streambuf_lock)
      : std::ostream(streambuf), ostream_guard_(*streambuf_lock) {
  }

  ~LogTransaction() {
    *this << std::endl;
  }

 private:
  std::lock_guard<std::mutex> ostream_guard_;
};

// Wrapper around std::ostream for writing log entries.
//
// C++ streams are not required to be thread-safe. This class protects
// an output stream with a mutex.
class Logger {
 public:
  explicit Logger(std::streambuf* streambuf) : streambuf_(streambuf) {
  }

  // TODO(ed): Should we add support for logging severities?
  LogTransaction Log() {
    return LogTransaction(streambuf_, &streambuf_lock_);
  }

 private:
  std::mutex streambuf_lock_;
  std::streambuf* streambuf_;
};

}  // namespace
}  // namespace util
}  // namespace flower

#endif
