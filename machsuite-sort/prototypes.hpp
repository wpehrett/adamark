#include <stdlib.h>
#include <list>

#ifndef PROTOTYPES_HPP
#define PROTOTYPES_HPP

//Start = signal ready to start simulation
//Done = signal to stop and check results
//GetData = request some amount of data be sent to the requesting module's input
//PutData = response to GetData request, or (unrequested) final output data to record for later checking. If sent by system with null data, this means all data from the input dataset has been ingested by the simulated system.
//FIXME edit for revised paradigm - make just Init/Start/Done (or, Init/Done) instead of separate GetData/PutData?
enum ThReqType {Start, Done, GetData, PutData};
class ThReqParams {
	public:
		ThReqType type;
		void* data; //note: must be freed (if applicable) by the message recipient to prevent memory leaks
		unsigned int datasize; //size in bytes; type assumed per testcase definition
		
		ThReqParams(ThReqType type, void* data, unsigned int datasize);
		~ThReqParams();
};

class Message {
	public:
		void* data; //note: must be freed (if applicable) by the message recipient to prevent memory leaks
		int src; //use -1 for system
		int dest; //use -1 for system
		//TODO later: add data size parameter (esp for power estimation) - maybe just as semi-optional metadata?
		//TODO later: add something to indicate data type? (Probably need to leave this purely user-defined, though)
		
		Message(void* data, int src, int dest);
		~Message();
};

class Module {
	public:
		unsigned int id; //global module identifier, used for message passing and addressing
		unsigned int latency; //number of cycles to traverse this module
		unsigned int cycleCount; //which cycle this module is on, bounded by [0, latency)
		unsigned int maxPipeline; //if this module is pipelined, how much can be in-flight at once (will likely == latency)
		bool stalled; //if waiting for data, set flag so this module won't advance until ready
		double cumulativeEnergy; //updated by simulator loop based on return value from process()
		
		/* TODO refine/revise: different signals going to/from different modules, ready and not-ready signals (and inputs of those signals from up/downstream modules), etc.. Probably: eliminate these altogether from the top-level Module class and make the programmer define them per their specific design... except that the top-level needs to know for the simulation cycle what gets passed where... */
		std::list<Message*> inputs;
		std::list<Message*> outputs;
		
		virtual void init(void* data, unsigned int datasize) = 0; //Called iff the module is defined as init in the system configuration file. Data size is given in bytes. TODO consider changing this to directly supply the vector and only the vector!
		
		virtual double process() = 0; //Returns energy expended in that simulation step, in nJ (TODO decide if that's an appropriate order of magnitude!); result may be zero (if not trying to model power), static (for simplicity), or variable (for greater accuracy) based on intra-module activity, lower value for cycles while stalled, etc.
};

#endif
