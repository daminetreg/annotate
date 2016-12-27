#include <iostream>
#include <cstdio>
#include <memory>
#include <boost/endian/buffers.hpp>  // see Synopsis below
#include <boost/static_assert.hpp>
#include <pre/bytes/utils.hpp>

#include <chrono>

using namespace boost::endian;

  namespace config {

    struct binary_output_config {

      /**
       * Duration of the Pulse signal (0 to 255ms) 
       */
      std::chrono::milliseconds pulse_duration{0};

      /**
       * Determine channel polarity, which will be used to interpret further channel values.
       */
      bool polarity{};

      /**
       * Value used by the rio in case nothing provided
       */
      bool safety_value{};
    };

    using binary_input_config = bool;
    using analog_output_value = uint8_t;

    struct remote_io {
      /**
       * Timeout that the device should wait for replies
       */
      std::chrono::seconds slc_timeout{10};

      /**
       * deadtime_timeout in 10th of seconds (1/10)
       */
      std::chrono::duration<int, std::deci> deadtime_timeout{10};

      /**
       * Time for the rio to startup
       */
      std::chrono::seconds powerup_timeout{1};
    };

    /**
     * Remote IO EY-EM510FXXX
     *
     * ![Mapping EY-EM510FXXX](../doc/diagrams/ey_em510fxx.png) 
     */
    struct ey_em510fxx : public remote_io {
      
      ey_em510fxx() : remote_io() {}

      binary_output_config triac_01{};
      binary_output_config triac_03{};
      binary_output_config triac_05{};

      binary_output_config relay_25{};
      binary_output_config relay_26{};
      binary_output_config relay_27{};

      binary_input_config ai_18{};
      binary_input_config ai_20{};
      binary_input_config ai_22{};
      binary_input_config ai_23{};

      analog_output_value ao_07{};
      analog_output_value ao_09{};
      analog_output_value ao_11{};

    };

  }

//  namespace serializer {
//      template <> BOOST_SYMBOL_EXPORT void remote_io_config_grammar<device::config::ey_em510fxx>::define_remote_io() {
//        using ka::byte_;
//
//        remote_io_ = 
//             _.init_bo_polarity
//          << _.clear_bo_safety_value
//          << _.binary_output
//          << _.binary_output
//          << _.binary_output
//          << _.binary_output
//          << _.binary_output
//          << _.binary_output
//          << byte_(phx::ref(_.bo_polarity_packer.packed))
//
//          << _.init_bi_polarity_at(2)
//          << _.binary_input
//          << _.binary_input
//          << _.binary_input
//          << _.binary_input
//          << byte_(phx::ref(_.bi_polarity_packer.packed))
//
//          << _.analog_output
//          << _.analog_output
//          << _.analog_output
//          << byte_(phx::ref(_.bo_safety_value_packer.packed))
//
//          << _.slc_timeout
//          << _.deadtime_timeout
//          << _.powerup_timeout;
//      }
//  }

struct alignas(1) bo_polarities_t {
  
  uint8_t reserved : 2; //XXX: Support writing 2_bits; ?

  bool triac_01_polarity : 1; 
  bool triac_03_polarity : 1; 
  bool triac_05_polarity : 1; 

  bool relay_25_polarity : 1; 
  bool relay_26_polarity : 1; 
  bool relay_27_polarity : 1; 
};

struct alignas(1) bo_safety_values_t {
  
  uint8_t reserved : 2; //XXX: Support writing 2_bits; ?

  bool triac_01_safety_value : 1; 
  bool triac_03_safety_value : 1; 
  bool triac_05_safety_value : 1; 

  bool relay_25_safety_value : 1; 
  bool relay_26_safety_value : 1; 
  bool relay_27_safety_value : 1; 
};



struct alignas(1) bi_polarities_t {
  
  uint8_t reserved : 2;

  bool ai_18 : 1; 
  bool ai_20 : 1; 
  bool ai_22 : 1; 
  bool ai_23 : 1; 

  uint8_t reserved_end : 2;
};

struct em510_binary_representation {
  big_int8_buf_t triac_01_pulse_duration;
  big_int8_buf_t triac_03_pulse_duration;
  big_int8_buf_t triac_05_pulse_duration;

  big_int8_buf_t relay_25_pulse_duration;
  big_int8_buf_t relay_26_pulse_duration;
  big_int8_buf_t relay_27_pulse_duration;

  endian_buffer<order::big, bo_polarities_t, sizeof(bo_polarities_t)*CHAR_BIT> bo_polarities;
  endian_buffer<order::big, bi_polarities_t, sizeof(bi_polarities_t)*CHAR_BIT> bi_polarities;

  big_int8_buf_t ao_07_safety_value;
  big_int8_buf_t ao_09_safety_value;
  big_int8_buf_t ao_11_safety_value;

  endian_buffer<order::big, bo_safety_values_t, sizeof(bo_safety_values_t)*CHAR_BIT> bo_safety_values;
};

const char* filename = "test.dat";

// Boost endian hack
namespace boost
{
namespace endian
{



  namespace detail
  {


    template<>
    struct unrolled_byte_loops<bo_polarities_t, 1, false>
    {
      typedef bo_polarities_t T;
      static T load_big(const unsigned char* bytes) BOOST_NOEXCEPT
        { return *reinterpret_cast<const T*>(bytes - 1); }
      static T load_little(const unsigned char* bytes) BOOST_NOEXCEPT
        { return *reinterpret_cast<const T*>(bytes); }
      static void store_big(char* bytes, T value) BOOST_NOEXCEPT
        { *(bytes - 1) = *(reinterpret_cast<char*>(&value)); }
      static void store_little(char* bytes, T value) BOOST_NOEXCEPT
        { *bytes = *(reinterpret_cast<char*>(&value)); }

    };

  }
}
}


int main(int, char* [])
{
  em510_binary_representation h;
  h.triac_01_pulse_duration = 12;
  bo_polarities_t pols;
  pols.triac_01_polarity = true;
  pols.relay_26_polarity = true;
  pols.relay_27_polarity = true;
  h.bo_polarities = pols; //XXX: THis is the main problem to solve first.

  std::string buffer (sizeof(h), char{});
  std::memcpy(const_cast<char*>(buffer.data()), &h, sizeof(h));

  std::cout << sizeof(h) << "-" << buffer.size() << " - " << pre::bytes::to_hexstring(buffer) << std::endl;

  //  Low-level I/O such as POSIX read/write or <cstdio>
  //  fread/fwrite is sometimes used for binary file operations
  //  when ultimate efficiency is important. Such I/O is often
  //  performed in some C++ wrapper class, but to drive home the
  //  point that endian integers are often used in fairly
  //  low-level code that does bulk I/O operations, <cstdio>
  //  fopen/fwrite is used for I/O in this example.

  std::FILE* fi = std::fopen(filename, "wb");  // MUST BE BINARY
  
  if (!fi)
  {
    std::cout << "could not open " << filename << '\n';
    return 1;
  }

  if (std::fwrite(&h, sizeof(em510_binary_representation), 1, fi)!= 1)
  {
    std::cout << "write failure for " << filename << '\n';
    return 1;
  }

  std::fclose(fi);

  std::cout << "created file " << filename << '\n';


  return 0;
}
