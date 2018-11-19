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
#include "Aes_subBytes.hpp"

using namespace std;

/* -------------------------------------------------------------------------- */
void aes_subBytes(uint8_t *buf)
{
    register uint8_t i = 16;

    sub : while (i--) buf[i] = rj_sbox(buf[i]);
} /* aes_subBytes */

//Constructor/destructor
Aes_subBytes::Aes_subBytes(unsigned int id) {
	this->id = id;
	cumulativeEnergy = 0.0;
}
Aes_subBytes::~Aes_subBytes() {}

void Aes_subBytes::init(void* data, unsigned int size) {}

double Aes_subBytes::process() {
	if (inputs.size() > 0) {
		//Note: expected input data format is a pointer to a Message with predetermined data size and format
		Message* m = inputs.front();
		uint8_t* arr = (uint8_t*)(m->data);
		
		aes_subBytes(arr);
		
		//Send to next
		//Programmatically, we just modified the message data, so let's tweak this message object and pass it on directly to save resources
		m->src = this->id;
		m->dest = 2; //the statically defined ID of our shiftRows module
		outputs.push_back(m);
		
		inputs.pop_front();
		
		return 0.7;
	}
	return 0.1;
}
