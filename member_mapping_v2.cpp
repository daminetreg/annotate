#include <iostream>
#include <utility>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/endian/buffers.hpp>  // see Synopsis below
#include <functional>
#include <array>

#include <boost/metaparse/string.hpp>
#include <boost/type_index.hpp>

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
constexpr size_t operator "" _bits(unsigned long long val) { return val; }
constexpr size_t operator "" _byte(unsigned long long val) { return val; }


  template <class T, const T>
  struct mapping {};


#define mapping_for(member_pointer) mapping<decltype(member_pointer), member_pointer>

/**
 * Rationale : The anchor_LINE typedef is made of a METAPARSE_STRING because this way we get an unique type for each
 *             field, based on their name. We cannot rely on the field member pointer, as it doesn't work for bitfields,
 *             and our first target for this member mapping library is binary serialization. And we want to map smaller than
 *             a byte.
 */ 
#define member_map(srcpath, dest, destpath)                                                                      \
  typedef BOOST_METAPARSE_STRING(BOOST_PP_STRINGIZE(srcpath)) BOOST_PP_CAT(anchor_ , __LINE__);                  \
  void fill(BOOST_PP_CAT(anchor_ , __LINE__), dest& d) const { d. destpath = srcpath; }                          \
  void update(BOOST_PP_CAT(anchor_ , __LINE__), const dest& d) { srcpath = d. destpath; }                        \



//template<const std::uintptr_t address>
//struct annotate_id {
//  static constexpr std::uintptr_t address = address;
//};
//
//#define annotate(T) annotate_id< __LINE__ >; T

/* 
 * binary serialization domain
 */
struct binary_representation {

  typedef binary_representation _self; // Need to be solved (as with qi grammar base_type ?)

  uint8_t pulse_for_triac01;
  uint8_t pulse_for_triac03;
  uint8_t pulse_for_triac05;


  struct alignas(1_byte) {
    
    uint8_t reserved                          : 2_bits;
    
    bool triac_01                             : 1_bits;
    bool triac_03                             : 1_bits;
    bool triac_05                             : 1_bits;

    uint8_t reserved_at_end                   : 3_bits;
  } bo_polarities;


  member_map(pulse_for_triac01, config::ey_em510fxx, triac_01.pulse_duration );
  member_map(pulse_for_triac03, config::ey_em510fxx, triac_03.pulse_duration );
  member_map(pulse_for_triac05, config::ey_em510fxx, triac_05.pulse_duration );
  member_map(bo_polarities.triac_01, config::ey_em510fxx, triac_01.polarity);
  member_map(bo_polarities.triac_03, config::ey_em510fxx, triac_03.polarity);
  member_map(bo_polarities.triac_05, config::ey_em510fxx, triac_05.polarity);


};

BOOST_FUSION_ADAPT_STRUCT(binary_representation,
  pulse_for_triac01,
  pulse_for_triac03,
  pulse_for_triac05,
  bo_polarities);


int main(int argc, char** argv) {

  std::cout << boost::typeindex::type_id_with_cvr<BOOST_METAPARSE_STRING("Hello")>() << std::endl;
 // auto now = (std::uintptr_t)(&binary_representation::pulse_for_triac01);

  config::ey_em510fxx internal{};

  std::cout << int(internal.triac_01.pulse_duration) << std::endl;
  
  binary_representation bin;

  std::cout << int(internal.triac_01.pulse_duration) << std::endl;

  std::cout << "sizeof(bin)" << sizeof(bin) << std::endl; 
//  static_assert(sizeof(bin) == 4, "TOO BIG");


  return 0;
}
