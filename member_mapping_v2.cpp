#include <iostream>
#include <utility>
#include <boost/preprocessor/cat.hpp>
#include <boost/endian/buffers.hpp>  // see Synopsis below
#include <functional>

using namespace boost::endian;


/*
 * Model domain 
 */

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

/*
 * Binary toolkit
 */


template <class T, class MappedField>
struct binary : public T {

  template<class A>
  constexpr auto ref_to_mapped(A& o) { return MappedField::get(o); }
};

#define BINARY_FIELD(SIZE, FIELD, MAPPING)                                     \
  struct BOOST_PP_CAT(FIELD, _mapping) {                                       \
    template <class T>                                                         \
    static auto get(T& o) { return std::ref(o. MAPPING ); }                    \
  };                                                                           \
  binary< big_int8_buf_t, BOOST_PP_CAT(FIELD, _mapping) > triac_01_pulse_duration;  \


/* 
 * binary serialization domain
 */
struct em510_config {

  struct some { template <class T> static auto get(T& o) { return std::ref(o.triac_01.pulse_duration); } };
  binary< big_int8_buf_t, some > triac_01_pulse_duration;

};

int main(int argc, char** argv) {
  config::ey_em510fxx internal{};

  std::cout << int(internal.triac_01.pulse_duration) << std::endl;
  
  em510_config cfg;
  cfg.triac_01_pulse_duration.ref_to_mapped(internal).get() = 120;

  std::cout << int(internal.triac_01.pulse_duration) << std::endl;

  static_assert(sizeof(cfg) == 1, "TOO BIG");


  return 0;
}
