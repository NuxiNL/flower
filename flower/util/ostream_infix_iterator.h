#ifndef FLOWER_UTIL_OSTREAM_INFIX_ITERATOR_H
#define FLOWER_UTIL_OSTREAM_INFIX_ITERATOR_H

#include <string>
#include <string_view>

namespace flower {
namespace util {
namespace {

// Class similar to std::ostream_iterator, except that the delimiter is
// only printed between elements. Unlike std::ostream_iterator, the
// final element is not followed by a delimiter.
template <class CharT = char, class Traits = std::char_traits<CharT>>
class ostream_infix_iterator {
 public:
  typedef std::basic_ostream<CharT, Traits> ostream_type;
  typedef std::basic_string_view<CharT, Traits> string_view_type;

  ostream_infix_iterator(ostream_type& stream, string_view_type delim)
      : stream_(&stream), delim_(delim), first_(true) {
  }

  ostream_infix_iterator& operator*() {
    return *this;
  }
  ostream_infix_iterator& operator++() {
    return *this;
  }
  ostream_infix_iterator& operator++(int) {
    return *this;
  }

  template <typename T>
  ostream_infix_iterator& operator=(const T& value) {
    if (!first_)
      *stream_ << delim_;
    first_ = false;
    *stream_ << value;
    return *this;
  }

 private:
  ostream_type* const stream_;
  const string_view_type delim_;
  bool first_;
};

}  // namespace
}  // namespace util
}  // namespace flower

#endif
