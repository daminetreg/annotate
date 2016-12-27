#include <iostream>

#include <cstdio>
#include <memory>
#include <pre/bytes/utils.hpp>

#include <chrono>

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


template <class T, class Field>
inline auto operator> (T&& lhs, Field rhs) { return lhs.*(rhs); }

// TODO: Implement type selectors like : 
//   -> all booleans
//   -> first / last T

int main(int argc, char** argv) {
  
  using namespace config;

  ey_em510fxx ecolinkconf;
  ecolinkconf.triac_01.polarity = true;

  auto some = ecolinkconf > &ey_em510fxx::triac_01 > &binary_output_config::polarity;

  std::cout << "Value : " << some << std::endl;
  
  // TODO: I would like to be able to say :
  //  map(ey_em510fxx.triac_01.polarity)
  //    .to(em510_binary_representation.bo_polarities.triac_01_polarity)

  //auto somes = ecolinkconf > all<binary_output_config>;
  //static_assert(std::is_same<decltype(somes), std::vector<binary_output_config ey_em510fxx::*>>::value);


//  map( [](auto ey_em510fxx) { return std::ref(ey_em510fxx.triac_01.polarity); })
//  .to(
//      [](auto em510_binary_representation) {
//        return std::ref(em510_binary_representation.bo_polarities.triac_01_polarity); 
//      }
//  );

  

  return 0;
}

//ey_em510fxx deserialize(em510_binary_representation bin) {
//
//  std::map<>
//
//  for (auto bin_ref_get : map) {
//    bin_ref_get.second(ey_em510fxx).get() = bin_ref_get.second(bin).get();
//  }
//}
