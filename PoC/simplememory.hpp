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
#define SIMPLEMEMORY_HPP
#endif

using namespace std;

//TODO: if possible: make all the various void*'s into some sort of disposable C++ object that can be garbage-collected

class MemReqParams {
	public:
		unsigned int address;
		unsigned int size;
		bool write;
		void* data; //note: must have been malloc'd!
		
		MemReqParams(unsigned int address, unsigned int size, bool write, void* data);
		~MemReqParams();
};

class SimpleMemory: public Module { //One port, arbitrary data, single-cycle latency, infinite i/o queue sizes
	public:
		void* mem; //Memory structure itself
		unsigned int size; //Size of the memory, in # data elements
		double energyPerCycle = 1.0; //can change this to be data-/request-/stall-dependent, if desired
		SimpleMemory(unsigned int id, unsigned int size);
		~SimpleMemory();
		
		double process();
};
