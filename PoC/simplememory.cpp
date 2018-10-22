#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <list>
#include <vector>
#include <algorithm>

//Required headers for simulator operation
#include "prototypes.hpp"

#include "simplememory.hpp"

using namespace std;

//MemReqParams constructor/destructor
MemReqParams::MemReqParams(unsigned int address, unsigned int size, bool write, void* data) {
	this->address = address;
	this->size = size;
	this->write = write;
	this->data = data; //Must be malloc'd, since the destructor frees it
}
MemReqParams::~MemReqParams() {
	free(data);
}

//SimpleMemory constructor/destructor
SimpleMemory::SimpleMemory(unsigned int id, unsigned int size) {
	this->id = id;
	latency = 1;
	cycleCount = 0;
	maxPipeline = 1;
	stalled = false;
	energyPerCycle = 0.0;
	cumulativeEnergy = 0.0;
	this->size = size;
	mem = (void*)calloc(size, 1); //Initialized to 0
	//Note: assumption of starting address = 0; this could be easily modified if desired e.g. to simulate virtual pages
	if (!mem) printf("Error: SimpleMemory constructor allocator returned null!\n");
}
SimpleMemory::~SimpleMemory() {
	free(mem);
}

//process(): store data from write requests and respond to read requests, in MemReqParams format; as with all process() functions, return the energy consumed in that cycle
double SimpleMemory::process() {
	if (inputs.size() > 0) {
		printf("SimpleMemory input size: %lu\n", inputs.size());
		printf("SimpleMemory output size: %lu\n", outputs.size());
		//Note: expected input data format is a pointer to a MemReqParams object
		Message* m = inputs.front();
		MemReqParams* r = (MemReqParams*)(m->data);
		unsigned int sender = m->src;
		
		if (r->write) { //write operation
			if ((r->address)+(r->size) > (this->size)) {
				printf("Error: SimpleMemory write op received with out-of-bounds params\n");
				delete r;
				inputs.pop_front();
				return 0;
			}
			if (!(r->data)) {
				printf("Error: SimpleMemory write op received with null data!\n");
				delete r;
				inputs.pop_front();
				return 0;
			}
			memcpy((void*)(mem+(r->address)), r->data, r->size);
			//free(r->data);
		}
		else { //read operation
			if ((r->address)+(r->size) > (this->size)) {
				printf("Error: SimpleMemory read op received with out-of-bounds params\n");
				delete r;
				inputs.pop_front();
				return 0;
			}
			void* data = malloc(r->size);
			if (!data) {
				printf("Error: SimpleMemory process() allocator returned null!\n");
				delete r;
				inputs.pop_front();
				return 0;
			}
			memcpy(data, (void*)(mem+(r->address)), r->size); //yes, this is pointer arithmetic...
			outputs.push_back(new Message((void*)data, this->id, sender));
			outputs.push_back(new Message((void*)(new ThReqParams(Done, NULL, 0)), this->id, sender));
		}
		delete r;
		delete m;
		inputs.pop_front();
		return energyPerCycle; //if op, add energy
	}
	return 0; //if no op, we're treating it as 0-energy for now
}
