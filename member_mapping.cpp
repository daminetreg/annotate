#include <iostream>
#include <utility>
#include <typeinfo>
#include <tuple>
#include <boost/endian/buffers.hpp>  // see Synopsis below

using namespace boost::endian;

namespace config {

  struct binary_output_config {

    /**
     * Duration of the Pulse signal (0 to 255ms) 
     */
    uint8_t pulse_duration{0};

    /**
     * Determine channel polarity, which will be used to interpret further channel values.
     */
    bool polarity{};

    /**
     * Value used by the rio in case nothing provided
     */
    bool safety_value{};
  };

  struct ey_em510fxx {
      
    binary_output_config triac_01{};
    binary_output_config triac_03{};
    binary_output_config triac_05{};
  
  };

}

template<class T, class... Members>
struct member_path : public std::tuple<Members...> {

//  auto operator()(const T& to_access) {
//    return to_access.*(Members...);
//  }
};


template <class T, class... Ts>
struct is_member_path : public std::false_type {};

template<class... Ts>
struct is_member_path<member_path<Ts...>> : public std::true_type {};

template<class T, class... Ts>
using disable_when_member_path = typename std::enable_if< !is_member_path<T>::value >::type*;


template<class T, class RHS, class RHST, disable_when_member_path<T>* = nullptr >
inline auto operator> (T&&, RHST RHS::* rhs) {
  return member_path<T, RHST RHS::*>{};
}

template<class... Ts, class RHS>
inline auto operator>(member_path<Ts...>&&, RHS) {
  return member_path<Ts..., RHS>{};
}

template <class T>
struct binary : public T {

  using T::operator=;

  template<class... Ts>
  binary<T>& operator=(member_path<Ts...>&&) {
    return *this;
  }
};

struct em510_config {

  binary<big_int8_buf_t> triac_01_pulse_duration;// = 
};


int main(int argc, char** argv) {
  auto test = config::ey_em510fxx{} > &config::ey_em510fxx::triac_01 > &config::binary_output_config::pulse_duration;

  test();

  // Conclusion : Field paths this way are not a so good idea, we could store them as Constant template parameters, but in the end I feel the best solution is to switch to a
  // lambda registration, as any lambda is a Type and can be referred as such, without costing anything in the type size.

  return 0;
}
