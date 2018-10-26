#ifndef __SSG_SAMPLE_T_HPP__
#define __SSG_SAMPLE_T_HPP__
#include <iostream>
#include <iomanip>

#define _SSG_TXT_OUTPUT_PRECISION_ 9

namespace synthetic_sample_generator {

using val_t = double;
using sample_t = std::pair<val_t, val_t>;


template <class T, bool>
struct print_pair : public std::unary_function<T, void> {};

template <class T>
struct print_pair<T, false>
{
  std::ostream& os;
  print_pair(std::ostream& strm) : os(strm) {}

  void operator()(const T& elem) const
  {
    os << std::setprecision(_SSG_TXT_OUTPUT_PRECISION_) << elem.first << ", " << elem.second << std::endl;
  }
};

template <class T>
struct print_pair<T, true>
{
  std::ostream& os;
  print_pair(std::ostream& strm) : os(strm) {}

  void operator()(const T& elem) const
  {
    os.write(&elem, sizeof(sample_t));
  }
};

} // end of namespce synthetic_sample_generator
#endif // __SSG_SAMPLE_T_HPP__
