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
#include "Aes256_encrypt_ecb.hpp"

using namespace std;

/* -------------------------------------------------------------------------- */
uint8_t rj_xtime(uint8_t x)
{
    return (x & 0x80) ? ((x << 1) ^ 0x1b) : (x << 1);
} /* rj_xtime */

/* -------------------------------------------------------------------------- */
void aes_addRoundKey(uint8_t *buf, uint8_t *key)
{
    register uint8_t i = 16;

    addkey : while (i--) buf[i] ^= key[i];
} /* aes_addRoundKey */

/* -------------------------------------------------------------------------- */
void aes_addRoundKey_cpy(uint8_t *buf, uint8_t *key, uint8_t *cpk)
{
    register uint8_t i = 16;

    cpkey : while (i--)  buf[i] ^= (cpk[i] = key[i]), cpk[16+i] = key[16 + i];
} /* aes_addRoundKey_cpy */

/* -------------------------------------------------------------------------- */
void aes_mixColumns(uint8_t *buf)
{
    register uint8_t i, a, b, c, d, e;

    mix : for (i = 0; i < 16; i += 4)
    {
        a = buf[i]; b = buf[i + 1]; c = buf[i + 2]; d = buf[i + 3];
        e = a ^ b ^ c ^ d;
        buf[i] ^= e ^ rj_xtime(a^b);   buf[i+1] ^= e ^ rj_xtime(b^c);
        buf[i+2] ^= e ^ rj_xtime(c^d); buf[i+3] ^= e ^ rj_xtime(d^a);
    }
} /* aes_mixColumns */

/* -------------------------------------------------------------------------- */
void aes_expandEncKey(uint8_t *k, uint8_t *rc)
{
    register uint8_t i;

    k[0] ^= rj_sbox(k[29]) ^ (*rc);
    k[1] ^= rj_sbox(k[30]);
    k[2] ^= rj_sbox(k[31]);
    k[3] ^= rj_sbox(k[28]);
    *rc = F( *rc);

    exp1 : for(i = 4; i < 16; i += 4)  k[i] ^= k[i-4],   k[i+1] ^= k[i-3],
        k[i+2] ^= k[i-2], k[i+3] ^= k[i-1];
    k[16] ^= rj_sbox(k[12]);
    k[17] ^= rj_sbox(k[13]);
    k[18] ^= rj_sbox(k[14]);
    k[19] ^= rj_sbox(k[15]);

    exp2 : for(i = 20; i < 32; i += 4) k[i] ^= k[i-4],   k[i+1] ^= k[i-3],
        k[i+2] ^= k[i-2], k[i+3] ^= k[i-1];

} /* aes_expandEncKey */

Aes256_encrypt_ecb::Aes256_encrypt_ecb(unsigned int id) {
	this->id = id;
	//latency = 15; //not actually cycles here, we're co-opting it to keep track of how many encryption rounds we're doing
	//cycleCount = 0;
	//maxPipeline = 1;
	stalled = false;
	cumulativeEnergy = 0.0;
	running = false;
	/*
		aes256_context ctx;
		uint8_t buf[16];
	*/
}
Aes256_encrypt_ecb::~Aes256_encrypt_ecb() {}

void Aes256_encrypt_ecb::init(void* data, unsigned int size) {
	if (data != NULL) {
		//The first 32 values are bytes of the encryption key, the next 16 are bytes to encrypt using that key (all actually uint8_t)
		//TODO eventually: un-hardcode #s
		for (int a=0; a<48; a++) {
			if (a < 32) this->ctx.key[a] = this->ctx.enckey[a] = this->ctx.deckey[a] = (uint8_t)(((int32_t*)data)[a]);
			else this->buf[a-32] = (uint8_t)(((int32_t*)data)[a]);
		}
		for (i = 8;--i;) {
			aes_expandEncKey(this->ctx.deckey, &rcon);
		}
	}
}

double Aes256_encrypt_ecb::process() {
	if (running) {
		if (inputs.size() > 0) { //Got a message; given the system configuration, should be from aes_shiftRows
			Message* m = inputs.front();
			inputs.pop_front();
			for (int a=0; a<16; a++) this->buf[a] = ((uint8_t*)(m->data))[a]; //not *technically* needed given how we're passing data around, but w/e
			if (i==14) {
				//DON'T do the last mixColumns step, just expandEncKey, addRoundKey, and signal testharness/send result
				aes_expandEncKey(this->ctx.key, &rcon);
				aes_addRoundKey(buf, this->ctx.key);
				//Signal testharness/send result; don't forget to delete the old Message object too!
				//Note that testharness expects an int32_t array rather than uint8_t, so we need to convert
				int32_t* result_arr = (int32_t*)malloc(16*sizeof(int32_t));
				for (int a=0; a<16; a++) result_arr[a] = (int32_t)buf[a];
				outputs.push_back(new Message((void*)(new ThReqParams(Done, (void*)result_arr, 16)), this->id, -1));
				delete m;
				running = false;
				return 0.0; //FIXME actual power
			}
			//If we get here, we're not done, so keep running...
			aes_mixColumns(this->buf);
			if (i & 1) aes_addRoundKey(buf, &this->ctx.key[16]);
			else aes_expandEncKey(this->ctx.key, &rcon), aes_addRoundKey(buf, this->ctx.key);
			//Send next to aes_subBytes; original call = "aes_subBytes(buf);"
			//Recall that we're just reusing the same Message object and only deleting it at the very end!
			m->data = this->buf;
			m->src = this->id;
			m->dest = 1; //statically defined subBytes module ID
			outputs.push_back(m);
			i++;
			return 0.0; //FIXME actual power
		}
		else { //Round not yet complete; keep waiting
			return 0.0; //Treat as idle for now; FIXME add some sort of power number here
		}
	}
	else if (inputs.size() > 0) {
		//Note: expected input data format is a pointer to a ThReqParams object, since we're not currently running
		Message* m = inputs.front();
		ThReqParams* r = (ThReqParams*)(m->data);
		unsigned int sender = m->src;
		
		if (r->type == Start) { //Start processing data currently in storage
			//cycleCount = 1;
			running = true;
			aes_addRoundKey_cpy(buf, (&ctx)->enckey, (&ctx)->key);
			i = 1;
			rcon = 1;
			//Send to subBytes to start running
			Message* m_out = new Message(buf, this->id, 1); //note that 1 is the statically defined ID of our subBytes module
			outputs.push_back(m_out);
		}
		else { //Unknown op, ignore
		}
		
		//Clean up message data objects that were malloc'd or created with C++ "new"
		delete r;
		delete m;
		inputs.pop_front();
		
		return 0.0; //FIXME change to energy value, if applicable
	}
	return 0.0; //if no op, we're treating it as 0-energy for now
}
