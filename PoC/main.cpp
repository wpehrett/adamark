//FIXME Warning: EVERYWHERE: ensure created objects are eventually delete'd ("delete <objname>;")!

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <list>
#include <vector>
#include <algorithm>

//Required headers for simulator operation
#include "prototypes.hpp"

//#include your custom module headers here
#include "simplememory.hpp"

using namespace std;

vector<Module*> modules;
unsigned int clkGlobal = 0;
bool verbose = 0;
void* data_output; //TODO make a dynamically resizing array of arbitrary/testcase-defined type
//TODO: make an abstract object template for "testcase" and then dynamically type all this stuff based upon the instantiated testcase's input/output/etc types?!?!

int cleanList() {
	//Sort modules vector by ID to improve efficiency of message delivery in sim
	sort(modules.begin(), modules.end(), [](const Module* a, const Module* b) {return a->id < b->id;});
	
	//Scan modules vector to ensure all (user-assigned) IDs are unique and strictly sequential; if not, message delivery would break
	for (int i=0; i<modules.size()-1; i++) {
		if ((modules[i]->id)+1 != modules[i+1]->id) return -1;
	}
	
	return 0;
}

void init() {
	//Set global input to first module
	//TODO generalize this
	int* testint = (int*)malloc(sizeof(int));
	*testint = 2;
	(*modules.begin())->inputs.push_back(new Message((void*)(new MemReqParams(0, sizeof(int), true, (void*)testint)), -1, 0));
	(*modules.begin())->inputs.push_back(new Message((void*)(new MemReqParams(0, sizeof(int), false, NULL)), -1, 0));
}

int step() {
	int result = 0;
	
	//Phase 1: for each module: process() per current inputs and update energy
	for (int i=0; i<modules.size(); i++) {
		Module* m = modules[i];
		if (verbose) printf("Processing module with id=%d...\n", m->id);
		m->cumulativeEnergy += m->process();
	}
	
	//Phase 2: interconnect: for each module: check outputs, propagate to next module's inputs (or global outputs)
	//TODO revise/generalize
	for (int i=0; i<modules.size(); i++) {
		Module* m = modules[i];
		if (verbose) printf("Examining outputs for module with id=%d...\n", m->id);
		while ((m->outputs).size() > 0) {
			Message* msg = (m->outputs).front();
			int src = msg->src;
			int dest = msg->dest;
			void* data = msg->data;
			if (verbose) printf("Found message: src=%d, dest=%d\n", src, dest);
			
			//Parse and handle messages to system
			if (dest == -1) {
				//Data type must be ThReqParams object
				ThReqParams* trp = (ThReqParams*)data;
				switch (trp->type) {
					case Start:
						//TODO
						break;
					case Done: //TODO: consider whether this is actually the best way of checking finish condition (vs if it'd be too weird to handle like this in a Module that's supposed to be representative of real hardware)! 
					/*(
					FIXME: A better way would probably be:
					1) do the event-based thing and then finish when the event queue is completely empty... implement as a linked-list with O(n) insertion and O(1) removal, where n=length of event queue, instead of the current module loop where every cycle is O(m) (where m=total number of total modules in the system)
						- Another potential benefit here: would permit simple same-cycle data xfer (if, e.g., want to simulate multiple submodules where some may feed data into others in the same cycle
					2) maybe do a hybrid where modules are scanned each cycle, but they can be skipped by setting a flag in a statically sized bitmask to tell whether they're stalled or on-delay until a later cycle? or have an m-sized int array where each value is the cycle on which the module will next need attention, then check that vs the current cycle count each sim step?
					So, the key questions to ask are:
					a) How many modules do we expect to have?
					b) How active are those modules on average?
					c) How long do we expect the event queues to become, and stretching how far into future time?
					d) If sticking with the check-every-cycle model, or if doing (2), how can the system supply a "done" signal without breaking the "correctness"/realism of the simulation?
						- Maybe a meta-module at the front- and back-end of the simulated system, user-defined, that are designed to do this checking and handle all direct testharness interactions?
					)*/
						//Finish after all remaining messages delivered
						result = -1;
						break;
					case GetData:
						//TODO
						break;
					case PutData:
						//TODO
						break;
				}
				delete trp;
				
				//result = data;
			}
			
			delete msg;
			(m->outputs).pop_front();
		}
	}
	
	//Step 3: update metrics per module activity
	//TODO later (if needed; i.e., if not done at top-level)
	
	return result;
}

int main(int argc, char* argv[]) {
	printf("Hi!\n\n");
	
	//Parse command-line args
	//TODO allow plaintext system configuration files, specification of particular tests or test subsets, etc.
	//TODO make the verbose flag a global, well-defined, well-documented parameter accessible by other, user-defined modules!
	for (int i=1; i<argc; i++) {
		if (strcmp(argv[i], "-v")==0 || strcmp(argv[i], "--verbose")==0) verbose=1;
	}
	
	//Init: instantiate system (TODO revise this part/make more user-friendly, and enable reading from a config file)
	SimpleMemory* sm = new SimpleMemory(0, 8); //Note: each module must take a unique sequential ID (!= -1) as a parameter and set it in its constructor
	modules.push_back(sm);
	
	if (cleanList() == -1) {
		printf("Error: problem with module IDs (non-unique and/or non-sequential)! Exiting.\n");
		return -1;
	}
	
	//Init: set up inputs (TODO parameterize somehow to more easily permit varying testcases; maybe just multiple different init() functions, incl stuff that can (wlog) stream large data from disk-stored files as needed?)
	printf("Initializing...\n");
	init();
	
	//Sim loop: step(), check if done after each step, break when done
	printf("Beginning simulation...\n");
	while(1) {
		if (verbose) printf("**** CYCLE %d ****\n", clkGlobal);
		int result = step();
		if (result == -1) {
			printf("Done signal detected\n\n");		
			break;
		}
		if (clkGlobal > 3) { if (verbose) printf("\n"); break; }
		clkGlobal++;
		if (verbose) printf("\n");
	}
	
	//Check results
	//TODO
	
	//Cleanup: compute final metrics, display results, exit
	printf("Simulation complete, cleaning up...\n");
	double totalEnergy = 0.0;
	for (int i=0; i<modules.size(); i++) {
		Module* m = modules[i];
		if (verbose) printf("Metrics for module with id=%d...\n", m->id);
		printf("Energy for module with id=%d: %.6f nJ\n", m->id, m->cumulativeEnergy);
		totalEnergy += m->cumulativeEnergy;
	}
	printf("Total energy: %.6f nJ\n", totalEnergy);
	printf("Total cycles: %d\n", clkGlobal+1); //note clkGlobal is 0-indexed
	
	delete sm;
	
	return 0;
}
/************************/
