#include <stdlib.h>

//Required headers for simulator operation
#include "prototypes.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "support.h"

#ifndef AESSHIFTROWS_HPP
#define AESSHIFTROWS_HPP

using namespace std;

class Aes_shiftRows: public Module {
	public:
		Aes_shiftRows(unsigned int id);
		~Aes_shiftRows();
		
		void init(void* data, unsigned int size);
		double process();
};

#endif
