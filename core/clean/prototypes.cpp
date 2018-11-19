#include <stdlib.h>
#include <list>
#include <stdio.h>

#include "prototypes.hpp"

ThReqParams::ThReqParams(ThReqType type, void* data, unsigned int datasize) {
	this->type = type;
	this->data = data;
	this->datasize = datasize;
}
ThReqParams::~ThReqParams() {}

Message::Message(void* data, int src, int dest) {
	this->data = data;
	this->src = src;
	this->dest = dest;
}
Message::~Message() {}

double FixedLatencyModule::process() {
	//Wait for message from another module containing some bundle of data
	//Pass message object directly on to process_real function at <latency> cycles in the future
	if (running) {
		if (cycleCount == latency) { //Done, send output msg
			running = false;
			cycleCount = 0;
			process_real(msg_tmp);
			delete msg_tmp;
			return activeEnergy;
		}
		else { //Not done, increment cycle count
			cycleCount++;
			return activeEnergy;
		}
	}
	else if (inputs.size() > 0) {
		msg_tmp = inputs.front(); //Only take the first message at a time for a fixed-latency module
		running = true;
		cycleCount = 1;
		inputs.pop_front();
		return activeEnergy;
	}
	return idleEnergy;
}
