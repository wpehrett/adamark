#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <list>
#include <vector>
#include <algorithm>

#ifndef PROTOTYPES_HPP
#define PROTOTYPES_HPP
#endif

using namespace std;

//TODO: if possible: make all the various void*'s into some sort of disposable C++ object that can be garbage-collected

class Message {
	public:
		void* data;
		int src; //use -1 for system
		int dest; //use -1 for system
		//TODO later: add data size parameter (esp for power estimation) - maybe just as semi-optional metadata?
		//TODO later: add something to indicate data type? (Probably need to leave this purely user-defined, though)
		
		//TODO make proper constructor/destructors
		Message(void* data, int src, int dest) { //Yes, this is bad style to define it here in a header file, but w/e
			this->data = data;
			this->src = src;
			this->dest = dest;
		}
		~Message() {}
};

class Module {
	public:
		unsigned int id; //global module identifier, used for message passing and addressing
		unsigned int latency; //number of cycles to traverse this module
		unsigned int cycleCount; //which cycle this module is on, bounded by [0, latency)
		unsigned int maxPipeline; //if this module is pipelined, how much can be in-flight at once (will likely == latency)
		bool stalled; //if waiting for data, set flag so this module won't advance until ready
		double cumulativeEnergy = 0.0; //updated by simulator loop; TODO break this out to at module-level to enable greater modeling detail (e.g. variable energy based on intra-module activity, lower value for cycles while stalled, etc.)
		
		/* TODO refine/revise: different signals going to/from different modules, ready and not-ready signals (and inputs of those signals from up/downstream modules), etc.. Probably: eliminate these altogether from the top-level Module class and make the programmer define them per their specific design... except that the top-level needs to know for the simulation cycle what gets passed where... */
		list<Message*> inputs;
		list<Message*> outputs;
		
		virtual double process() = 0; //Returns energy expended in that simulation step, in nJ (TODO decide if that's an appropriate order of magnitude!)
		
		//TODO make prooper constructor/destructors
		Module() {}
		~Module() {}
};
