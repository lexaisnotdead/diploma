#include <cstdint>
#include <atomicassets/zigzag.hpp>

uint64_t zigzag_encode(int64_t value) {
   if (value < 0) {
      return ((uint64_t)(- (value + 1)) << 1) + 1;
   } else {
      return (uint64_t) value << 1;
   }
}

int64_t zigzag_decode(uint64_t value) {
   if (value % 2 == 0) {
      return (int64_t)(value >> 1);
   } else {
      return -((int64_t)(value >> 1) + 1);
   }
}