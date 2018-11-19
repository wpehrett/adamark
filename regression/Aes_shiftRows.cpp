#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <list>
#include <vector>
#include <algorithm>

//Required headers for simulator operation
#include "prototypes.hpp"

#include "aes256_common.hpp"
#include "Aes_shiftRows.hpp"

using namespace std;

/* -------------------------------------------------------------------------- */
void aes_shiftRows(uint8_t *buf)
{
    register uint8_t i, j; /* to make it potentially parallelable :) */

    i = buf[1]; buf[1] = buf[5]; buf[5] = buf[9]; buf[9] = buf[13]; buf[13] = i;
    i = buf[10]; buf[10] = buf[2]; buf[2] = i;
    j = buf[3]; buf[3] = buf[15]; buf[15] = buf[11]; buf[11] = buf[7]; buf[7] = j;
    j = buf[14]; buf[14] = buf[6]; buf[6]  = j;

} /* aes_shiftRows */

//Constructor/destructor
Aes_shiftRows::Aes_shiftRows(unsigned int id) {
	this->id = id;
	cumulativeEnergy = 0.0;
}
Aes_shiftRows::~Aes_shiftRows() {}

void Aes_shiftRows::init(void* data, unsigned int size) {}

double Aes_shiftRows::process() {
	if (inputs.size() > 0) {
		//Note: expected input data format is a pointer to a Message with predetermined data size and format
		Message* m = inputs.front();
		uint8_t* arr = (uint8_t*)(m->data);
		
		aes_shiftRows(arr);
		
		//Send to next
		//Programmatically, we just modified the message data, so let's tweak this message object and pass it on directly to save resources
		m->src = this->id;
		m->dest = 0; //the statically defined ID of our encrypt_ecb module
		outputs.push_back(m);
		
		inputs.pop_front();
		
		return 0.65;
	}
	return 0.1;
}
