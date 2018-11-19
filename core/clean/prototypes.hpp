#include <stdlib.h>
#include <list>

#ifndef PROTOTYPES_HPP
#define PROTOTYPES_HPP

//Start = signal ready to start simulation
//Done = signal to stop and check results
//GetData = request some amount of data be sent to the requesting module's input (FIXME currently unused)
//PutData = response to GetData request (FIXME currently unused)
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
		//Note: data size parameter not currently included because we may be sending objects with inherent size info as "data"
		
		Message(void* data, int src, int dest);
		~Message();
};

class Module {
	public:
		unsigned int id; //global module identifier, used for message passing and addressing
		double cumulativeEnergy; //updated by simulator loop based on return value from process(); should not be directly modified by module internals
		
		std::list<Message*> inputs;
		std::list<Message*> outputs;
		
		virtual void init(void* data, unsigned int datasize) = 0; //Called iff the module is defined as init in the system configuration file. Data size is given in bytes.
		
		virtual double process() = 0; //Returns energy expended in that simulation step, in nJ
};

class FixedLatencyModule: public Module {
	public: //TODO revise/revine access permissions for these (public/private/protected) - all these can *probably* be protected
		unsigned int latency;
		unsigned int cycleCount;
		
		Message* msg_tmp;
		
		double idleEnergy;
		double activeEnergy;
		
		bool running;
		bool stalled; //FIXME not currently used/implemented
		
		double process();
		
		virtual void process_real(Message* m) = 0;
};

#endif
