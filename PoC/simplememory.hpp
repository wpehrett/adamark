#include <stdlib.h>

//Required headers for simulator operation
#include "prototypes.hpp"

#ifndef SIMPLEMEMORY_HPP
#define SIMPLEMEMORY_HPP

using namespace std;

class MemReqParams {
	public:
		unsigned int address; //Starting address for the read/write request
		unsigned int size; //Size in bytes of the data being read/written
		bool write;
		void* data; //Must be malloc'd, since the destructor (in simplememory.cpp) frees it
		
		MemReqParams(unsigned int address, unsigned int size, bool write, void* data);
		~MemReqParams();
};

class SimpleMemory: public Module { //One port, arbitrary data, single-cycle latency, infinite i/o queue sizes
	public:
		void* mem; //Memory structure itself
		unsigned int size; //Size of the memory, in # bytes
		double energyPerCycle; //can change this to be data-/request-/stall-dependent, if desired
		
		SimpleMemory(unsigned int id, unsigned int size);
		~SimpleMemory();
		
		double process();
};

#endif
