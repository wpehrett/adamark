#include <stdlib.h>
#include <list>

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
