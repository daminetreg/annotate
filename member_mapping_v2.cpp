#include <iostream>
#include <utility>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/seq.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/punctuation/remove_parens.hpp>
#include <boost/preprocessor/facilities/expand.hpp>
#include <boost/mpl/range_c.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/endian/buffers.hpp>  // see Synopsis below
#include <functional>
#include <array>

#include <boost/type_index.hpp>

using namespace boost::endian;



/*
 * Binary toolkit
 */
constexpr size_t operator "" _bits(unsigned long long val) { return val; }
constexpr size_t operator "" _byte(unsigned long long val) { return val; }


  template <class T, const T>
  struct mapping {};


#define mapping_for(member_pointer) mapping<decltype(member_pointer), member_pointer>

#define member_map(id, srcpath, destpath)                                                                      \
  typedef std::integral_constant<size_t, id> BOOST_PP_CAT(anchor_ , id);                  \
  void fill(BOOST_PP_CAT(anchor_ , id), const src_type& s, dest_type& d) const { d. destpath = s. srcpath; }                          \
  void update(BOOST_PP_CAT(anchor_ , id), src_type& s, const dest_type& d) { s. srcpath = d. destpath; }                        \

template <class SRC, class DEST>
struct member_mapping : public std::false_type {};

#define MEMBER_MAPPINGS_ON_EACH(r, data, elem) \
  member_map( r,  BOOST_PP_TUPLE_ELEM( 2, 0, elem), BOOST_PP_TUPLE_ELEM(2, 1, elem) )

#define MAKE_ELEM_ID(r, data, elem) \
  , BOOST_PP_CAT(anchor_ , r)

   
#define map_to(SRC_TYPE, DEST_TYPE, MAPPINGS)                   \
  template<>                                                    \
  struct member_mapping<SRC_TYPE, DEST_TYPE> {                  \
                                                                \
    typedef SRC_TYPE src_type;                                  \
    typedef DEST_TYPE dest_type;                                \
                                                                \
                                                                \
    typedef boost::mpl::range_c<size_t, 2, 1 + BOOST_PP_SEQ_SIZE(MAPPINGS)> mappings; \
                                                                \
    BOOST_PP_SEQ_FOR_EACH(MEMBER_MAPPINGS_ON_EACH, _, MAPPINGS )    \
  };                                                            \













/*
 * -------------------------- USER code Model domain -----------------------------------
 */



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
 * -------------------------- USER BINARY SERIALIZATION CODE -----------------------------------
 */

struct binary_representation {

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

  friend struct member_mapping<binary_representation, config::ey_em510fxx>;
};

map_to(binary_representation, config::ey_em510fxx,
  ((pulse_for_triac01, triac_01.pulse_duration))
  ((pulse_for_triac03, triac_03.pulse_duration))
  ((pulse_for_triac05, triac_05.pulse_duration))
  ((bo_polarities.triac_01, triac_01.polarity))
  ((bo_polarities.triac_03, triac_03.polarity))
  ((bo_polarities.triac_05, triac_05.polarity))
);



























int main(int argc, char** argv) {

  config::ey_em510fxx internal{};

  std::cout << int(internal.triac_01.pulse_duration) << std::endl;
  
  binary_representation bin;

  std::cout << int(internal.triac_01.pulse_duration) << std::endl;

  std::cout << "sizeof(bin)" << sizeof(bin) << std::endl; 
  static_assert(sizeof(bin) == 4, "TOO BIG");


  return 0;
}
