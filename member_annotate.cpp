#include <iostream>
#include <utility>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/seq.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/punctuation/remove_parens.hpp>
#include <boost/preprocessor/facilities/expand.hpp>
#include <boost/preprocessor/variadic/elem.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>
#include <boost/mpl/list.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/endian/buffers.hpp>  // see Synopsis below
#include <functional>
#include <array>

#include <boost/metaparse/string.hpp>
//#include <boost/type_index.hpp>


/*
 * FRAMEWORK CODE for the Binary toolkit
 */
constexpr size_t operator "" _bits(unsigned long long val) { return val; }
constexpr size_t operator "" _byte(unsigned long long val) { return val; }

/**
 * Rationale : The anchor_LINE typedef is made of a METAPARSE_STRING because this way we get an unique type for each
 *             field, based on their name. We cannot rely on the field member pointer, as it doesn't work for bitfields,
 *             and our first target for this member mapping library is binary serialization. And we want to map smaller than
 *             a byte.
 *
 */ 

#define get_annotations_on_each(r, data, elem) , elem


#define 📜(field, ...)                                                                                                      \
  auto get_annotations( BOOST_METAPARSE_STRING(BOOST_PP_STRINGIZE(field)) ) { \
                                                                                                                     \
    return std::make_tuple(                                                                                                         \
       bool{} BOOST_PP_SEQ_FOR_EACH(get_annotations_on_each, unused,                                                   \
       BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__) )                    \
    );                                                                                                               \
  }


#define annotated_on_each(r, data, elem) , BOOST_METAPARSE_STRING(BOOST_PP_STRINGIZE(elem))

#define annotated(...) \
  typedef boost::mpl::list< bool \
    BOOST_PP_SEQ_FOR_EACH(annotated_on_each, unused, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)) \
  > annotated;


struct jsonize {};


template<class srcpath, class dstpath>
struct map_to {
  std::function<srcpath> fill_src;
  std::function<dstpath> fill_dst;
};

#define member_map(srcpath, dsttype, dstpath)                                     \
  (map_to< void( decltype(*this)& src, const dsttype& dst ) , void( decltype(*this)& src, dsttype& dst ) > {\
    []( decltype(*this)& src, const dsttype& dst ) { src. srcpath = dst. dstpath; }, \
    []( decltype(*this)& src, dsttype& dst ) { dst. dstpath = src. srcpath; } \
  })


















/*
 * -------------------------- USER code Model domain -----------------------------------
 */

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



/*
 * -------------------------- USER BINARY SERIALIZATION CODE -----------------------------------
 */

struct binary_representation {
  annotated(pulse_for_triac01, pulse_for_triac03, bo_polarities)

  📜(pulse_for_triac01,
      member_map(pulse_for_triac01, config::ey_em510fxx, triac_01.pulse_duration) , jsonize{} )
  uint8_t pulse_for_triac01;

  📜(pulse_for_triac03, 
      member_map(pulse_for_triac01, config::ey_em510fxx, triac_03.pulse_duration), jsonize{} )

  uint8_t pulse_for_triac03;

  📜(pulse_for_triac05,
      member_map(pulse_for_triac01, config::ey_em510fxx, triac_05.pulse_duration) )
  uint8_t pulse_for_triac05;


  struct alignas(1_byte) {
    annotated(triac_01, triac_03, triac_05)
    
    uint8_t reserved                          : 2_bits;
    
    bool triac_01                             : 1_bits;
    bool triac_03                             : 1_bits;
    bool triac_05                             : 1_bits;

    📜(triac_01, member_map(triac_01, config::ey_em510fxx, triac_01.polarity) , jsonize{} )
    📜(triac_03, member_map(triac_03, config::ey_em510fxx, triac_03.polarity) )
    📜(triac_05, member_map(triac_05, config::ey_em510fxx, triac_05.polarity) )

    uint8_t reserved_at_end                   : 3_bits;

  } bo_polarities;

};



























int main(int argc, char** argv) {

  config::ey_em510fxx internal{};

  std::cout << int(internal.triac_01.pulse_duration) << std::endl;
  
  binary_representation bin;

  // std::cout << typeid(bin.get_annotations(BOOST_METAPARSE_STRING("pulse_for_triac01"){})).name() << std::endl; 

  bin.pulse_for_triac01 = 120;

  std::get<1>(bin.get_annotations(BOOST_METAPARSE_STRING("pulse_for_triac01"){})).fill_dst(bin, internal);

  std::cout << int(internal.triac_01.pulse_duration) << std::endl;

  std::cout << "sizeof(bin)" << sizeof(bin) << std::endl; 
  static_assert(sizeof(bin) == 4, "TOO BIG");


  return 0;
}
