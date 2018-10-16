#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <list>
#include <vector>
#include <algorithm>

//Required headers for simulator operation
#ifndef PROTOTYPES_HPP
#include "prototypes.hpp"
#endif

#ifndef SIMPLEMEMORY_HPP
#include "simplememory.hpp"
#endif

using namespace std;

//TODO: if possible: make all the various void*'s into some sort of disposable C++ object that can be garbage-collected

//MemReqParams constructor/destructor
MemReqParams::MemReqParams(unsigned int address, unsigned int size, bool write, void* data) {
	this->address = address;
	this->size = size;
	this->write = write;
	this->data = data;
}
MemReqParams::~MemReqParams() {}

//SimpleMemory constructor/destructor
SimpleMemory::SimpleMemory(unsigned int id, unsigned int size) {
	this->id = id;
	latency = 1;
	cycleCount = 0;
	maxPipeline = 1;
	stalled = false;
	cumulativeEnergy = 0.0;
	this->size = size;
	mem = (void*)malloc(size); //Note: uninitialized!
	//Note: assumption of starting address = 0; this could be easily modified if desired e.g. to simulate virtual pages
	if (!mem) printf("Error: SimpleMemory constructor allocator returned null!\n");
}
SimpleMemory::~SimpleMemory() {
	free(mem);
}

//process(): store data from write requests and respond to read requests, in MemReqParams format; as with all process() functions, return the energy consumed in that cycle
double SimpleMemory::process() {
	if (inputs.size() > 0) {
		//Note: expected input data format is a pointer to a struct of two ints: [address, size]
		Message* m = inputs.front();
		inputs.pop_front();
		MemReqParams* r = (MemReqParams*)(m->data);
		unsigned int sender = m->src;
		
		if (r->write) { //write operation
			if (!(r->data)) {
				printf("Error: SimpleMemory write op received with null data!\n");
				return 0;
			}
			if ((r->address)+(r->size) > (this->size)) {
				printf("Error: SimpleMemory write op received with out-of-bounds params\n");
				return 0;
			}
			memcpy((void*)(mem+(r->address)), r->data, r->size);
			free(r->data);
		}
		else { //read operation
			void* data = malloc(r->size);
			if (!data) {
				printf("Error: SimpleMemory process() allocator returned null!\n");
				return 0;
			}
			if ((r->address)+(r->size) > (this->size)) {
				printf("Error: SimpleMemory read op received with out-of-bounds params\n");
				return 0;
			}
			memcpy(data, (void*)(mem+(r->address)), r->size); //yes, this is pointer arithmetic...
			outputs.push_back(new Message((void*)data, this->id, sender));
		}
		return energyPerCycle; //if op, add energy
	}
	return 0; //if no op, we're treating it as 0-energy for now
}
