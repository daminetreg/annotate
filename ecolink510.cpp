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

  bool triac_01 : 1; 
  bool triac_03 : 1; 
  bool triac_05 : 1; 

  bool relay_25 : 1; 
  bool relay_26 : 1; 
  bool relay_27 : 1; 
};

struct alignas(1) bo_safety_values_t {
  
  uint8_t reserved : 2; //XXX: Support writing 2_bits; ?

  bool triac_01 : 1; 
  bool triac_03 : 1; 
  bool triac_05 : 1; 

  bool relay_25 : 1; 
  bool relay_26 : 1; 
  bool relay_27 : 1; 
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

  em510_binary_representation() {}
  em510_binary_representation(config::ey_em510fxx& src) {
    triac_01_pulse_duration = src.triac_01.pulse_duration.count();
    triac_03_pulse_duration = src.triac_03.pulse_duration.count();
    triac_05_pulse_duration = src.triac_05.pulse_duration.count();

    relay_25_pulse_duration = src.relay_25.pulse_duration.count();
    relay_26_pulse_duration = src.relay_26.pulse_duration.count();
    relay_27_pulse_duration = src.relay_27.pulse_duration.count();

    { bo_polarities_t p;
      p.triac_01 = src.triac_01.polarity;
      p.triac_03 = src.triac_03.polarity;
      p.triac_05 = src.triac_05.polarity;

      p.relay_25 = src.relay_25.polarity;
      p.relay_26 = src.relay_26.polarity;
      p.relay_27 = src.relay_27.polarity;
      bo_polarities = p;
    }

    { bi_polarities_t p;
      p.ai_18 = src.ai_18;
      p.ai_20 = src.ai_20;
      p.ai_22 = src.ai_22;
      p.ai_23 = src.ai_23;
      bi_polarities = p;
    }

    ao_07_safety_value = src.ao_07;
    ao_09_safety_value = src.ao_09;
    ao_11_safety_value = src.ao_11;

    { bo_safety_values_t s;
      s.triac_01 = src.triac_01.safety_value;
      s.triac_03 = src.triac_03.safety_value;
      s.triac_05 = src.triac_05.safety_value;

      s.relay_25 = src.relay_25.safety_value;
      s.relay_26 = src.relay_26.safety_value;
      s.relay_27 = src.relay_27.safety_value;

      bo_safety_values = s;
    }

  }

  operator config::ey_em510fxx () {
    config::ey_em510fxx dst;

    dst.triac_01.pulse_duration = std::chrono::milliseconds{triac_01_pulse_duration.value()};
    dst.triac_03.pulse_duration = std::chrono::milliseconds{triac_03_pulse_duration.value()};
    dst.triac_05.pulse_duration = std::chrono::milliseconds{triac_05_pulse_duration.value()};

    dst.relay_25.pulse_duration = std::chrono::milliseconds{relay_25_pulse_duration.value()};
    dst.relay_26.pulse_duration = std::chrono::milliseconds{relay_26_pulse_duration.value()};
    dst.relay_27.pulse_duration = std::chrono::milliseconds{relay_27_pulse_duration.value()};

    { bo_polarities_t p = bo_polarities.value();
       dst.triac_01.polarity = p.triac_01;
       dst.triac_03.polarity = p.triac_03;
       dst.triac_05.polarity = p.triac_05;

       dst.relay_25.polarity = p.relay_25;
       dst.relay_26.polarity = p.relay_26;
       dst.relay_27.polarity = p.relay_27;
    }

    { bi_polarities_t p = bi_polarities.value();
      dst.ai_18 = p.ai_18; 
      dst.ai_20 = p.ai_20; 
      dst.ai_22 = p.ai_22; 
      dst.ai_23 = p.ai_23; 
    }

    dst.ao_07 = ao_07_safety_value.value();
    dst.ao_09 = ao_09_safety_value.value();
    dst.ao_11 = ao_11_safety_value.value();

    { bo_safety_values_t s;

      dst.triac_01.safety_value = s.triac_01;
      dst.triac_03.safety_value = s.triac_03;
      dst.triac_05.safety_value = s.triac_05;
      dst.relay_25.safety_value = s.relay_25;
      dst.relay_26.safety_value = s.relay_26;
      dst.relay_27.safety_value = s.relay_27;
    }

    return dst;
  }

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

    template<>
    struct unrolled_byte_loops<bi_polarities_t, 1, false>
    {
      typedef bi_polarities_t T;
      static T load_big(const unsigned char* bytes) BOOST_NOEXCEPT
        { return *reinterpret_cast<const T*>(bytes - 1); }
      static T load_little(const unsigned char* bytes) BOOST_NOEXCEPT
        { return *reinterpret_cast<const T*>(bytes); }
      static void store_big(char* bytes, T value) BOOST_NOEXCEPT
        { *(bytes - 1) = *(reinterpret_cast<char*>(&value)); }
      static void store_little(char* bytes, T value) BOOST_NOEXCEPT
        { *bytes = *(reinterpret_cast<char*>(&value)); }

    };

    template<>
    struct unrolled_byte_loops<bo_safety_values_t, 1, false>
    {
      typedef bo_safety_values_t T;
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

  config::ey_em510fxx mycfg;
  mycfg.triac_01.safety_value = true;
  mycfg.triac_03.polarity = true;
  mycfg.ai_23 = true;
  //mycfg.slc_timeout = std::chrono::seconds{15}; 

  em510_binary_representation h{mycfg};

  std::string buffer (sizeof(h), char{});
  std::memcpy(const_cast<char*>(buffer.data()), &h, sizeof(h));

  std::cout << sizeof(h) << "-" << buffer.size() << " - " << pre::bytes::to_hexstring(buffer) << std::endl;

  em510_binary_representation deser{mycfg};
  std::memcpy(&deser, buffer.data(), sizeof(h));
  
  config::ey_em510fxx desered = deser;

  assert(desered.triac_01.safety_value == mycfg.triac_01.safety_value);
  assert(desered.triac_03.polarity == mycfg.triac_03.polarity);
  assert(desered.ai_23 == mycfg.ai_23);
  //assert(desered.slc_timeout == mycfg.slc_timeout);



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
