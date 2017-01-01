#include <iostream>

struct nibbles_t {

  uint8_t low : 4;
  uint8_t high : 4;
};

int main(int argc, char** argv) {

  nibbles_t nibbles;
  
  nibbles.low = 0xC;
  nibbles.high = 0xE;

  auto set_low = [](nibbles_t& nibbles, auto val) { nibbles.low = val; };
  auto get_low = [](nibbles_t& nibbles) { return nibbles.low; };

  std::cout << unsigned(get_low(nibbles)) << std::endl;
  set_low(nibbles, 0xD);
  std::cout << unsigned(get_low(nibbles)) << std::endl;
  return 0;
}
