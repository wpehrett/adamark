#include <stdlib.h>

//Required headers for simulator operation
#include "prototypes.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "support.h"

#ifndef AESSUBBYTES_HPP
#define AESSUBBYTES_HPP

using namespace std;

class Aes_subBytes: public Module {
	public:
		Aes_subBytes(unsigned int id);
		~Aes_subBytes();
		
		void init(void* data, unsigned int size);
		double process();
};

#endif
