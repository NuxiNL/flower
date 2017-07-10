// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// This file is distributed under a 2-clause BSD license.
// See the LICENSE file for details.

#ifndef FLOWER_UTIL_NULL_OSTREAM_H
#define FLOWER_UTIL_NULL_OSTREAM_H

#include <ostream>
#include <streambuf>

namespace flower {
namespace util {
namespace {

// Stream buffer that discards any data to be written.
class null_streambuf : public std::streambuf {
 public:
  std::streamsize xsputn(const char* s, std::streamsize n) override {
    return n;
  }

  int overflow(int c) override {
    return traits_type::to_int_type(c);
  }
};

// Output stream that discards any data to be written.
class null_ostream : public std::ostream {
 public:
  null_ostream() : std::ostream(&buf_) {
  }

 private:
  null_streambuf buf_;
};

}  // namespace
}  // namespace util
}  // namespace flower

#endif
