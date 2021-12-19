/*Random number generator;
  Linear congruential generator
  from Numerical Recipes by Press et al.
  Jonathan Valvano

  How to use:
  Call Random_Init once with a seed. For example
      Random_Init(1);
	or
      Random_Init(NVIC_CURRENT_R);
  Call Random over and over to get a new random number. For example
      n = Random();      // returns a random 8-bit number
      m = Random16()%60; // returns a random number from 0 to 59
*/
#include <stdint.h>
static uint32_t M=1;
#define SLOPE 1664525
#define OFFSET 1013904223
void Random_Init(uint32_t seed){
  M = seed;
}

uint32_t Random(void){    // 0 to 255, very random
  M = SLOPE*M+OFFSET;     // 1664525*M+1013904223
  return (M>>24);
}

uint32_t Random16(void){  // 0 to 65535, sort of random
  M = SLOPE*M+OFFSET;     // 1664525*M+1013904223
  return (M>>16);
}
uint32_t Random32(void){  // 32-bit, but not very random
  M = SLOPE*M+OFFSET;     // 1664525*M+1013904223
  return M;
}

