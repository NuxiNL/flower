// Copyright (c) 2017 Nuxi, https://nuxi.nl/
//
// SPDX-License-Identifier: BSD-2-Clause

#ifndef FLOWER_UTIL_FD_STREAMBUF_H
#define FLOWER_UTIL_FD_STREAMBUF_H

#include <unistd.h>

#include <memory>
#include <streambuf>

#include <arpc++/arpc++.h>

namespace flower {
namespace util {
namespace {

// Stream buffer that writes data to a file descriptor.
// TODO(ed): Add buffering.
class fd_streambuf : public std::streambuf {
 public:
  explicit fd_streambuf(const std::shared_ptr<arpc::FileDescriptor>& fd)
      : fd_(fd) {
  }

  int overflow(int c) override {
    char ch = c;
    if (write(fd_->get(), &ch, 1) != 1)
      return traits_type::eof();
    return traits_type::to_int_type(c);
  }

  std::streamsize xsputn(const char* s, std::streamsize n) override {
    return write(fd_->get(), s, n);
  }

 private:
  std::shared_ptr<arpc::FileDescriptor> fd_;
};

}  // namespace
}  // namespace util
}  // namespace flower

#endif
