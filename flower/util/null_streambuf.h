// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// SPDX-License-Identifier: BSD-2-Clause

#ifndef FLOWER_UTIL_NULL_STREAMBUF_H
#define FLOWER_UTIL_NULL_STREAMBUF_H

#include <streambuf>

namespace flower {
namespace util {
namespace {

// Stream buffer that discards any data to be written.
class null_streambuf : public std::streambuf {
 public:
  int overflow(int c) override {
    return traits_type::to_int_type(c);
  }

  std::streamsize xsputn(const char* s, std::streamsize n) override {
    return n;
  }
};

}  // namespace
}  // namespace util
}  // namespace flower

#endif
