#include <stdlib.h>

//Required headers for simulator operation
#include "prototypes.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "support.h"

#ifndef AES256ENCRYPTECB_HPP
#define AES256ENCRYPTECB_HPP

using namespace std;

typedef struct {
  uint8_t key[32];
  uint8_t enckey[32];
  uint8_t deckey[32];
} aes256_context;

class Aes256_encrypt_ecb: public Module {
	public:
		aes256_context ctx;
		uint8_t buf[16];
		
		bool running;
		
    uint8_t rcon = 1;
    uint8_t i;
		
		Aes256_encrypt_ecb(unsigned int id);
		~Aes256_encrypt_ecb();
		
		void init(void* data, unsigned int size);
		
		double process();
};

#endif
