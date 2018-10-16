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

//#include your custom module headers here
#ifndef SIMPLEMEMORY_HPP
#include "simplememory.hpp"
#endif

using namespace std;

//TODO: if possible: make all the various void*'s into some sort of disposable C++ object that can be garbage-collected

vector<Module*> modules;
unsigned int clkGlobal = 0;
bool verbose = 0;

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
	//TODO make non-hacky!!!!!
	//TODO set up a means of including streamed data in the sim cycle
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
	//TODO revise/generify
	for (int i=0; i<modules.size(); i++) {
		Module* m = modules[i];
		if (verbose) printf("Examining outputs for module with id=%d...\n", m->id);
		list<Message*> o = m->outputs;
		for (list<Message*>::iterator it_o=o.begin(); it_o != o.end(); it_o++) {
			int src = (*it_o)->src;
			int dest = (*it_o)->dest;
			int data = *(int*)((*it_o)->data);
			if (verbose) printf("Found message: src=%d, dest=%d, data=%d\n", src, dest, data);
			if (dest == -1) result = data; //message is to system, record and return
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
		if (result != 0) { //TODO revise this to be more generic, definable via testcase
			printf("Concluding: result code = %d\n\n", result);		
			break;
		}
		clkGlobal++;
		if (verbose) printf("\n");
	}
	
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
	
	return 0;
}
/************************/
