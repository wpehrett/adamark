#include <stdlib.h>

//Required headers for simulator operation
#include "prototypes.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "support.h"

#define SIZE 2048
#define TYPE int32_t
#define TYPE_MAX INT32_MAX

#ifndef MSMERGESORT_HPP
#define MSMERGESORT_HPP

using namespace std;

class Ms_MergeSort: public Module { //One port, arbitrary data, single-cycle latency, infinite i/o queue sizes
	public:
		TYPE data[SIZE]; //Memory structure itself
		
		bool running;
		
		Ms_MergeSort(unsigned int id);
		~Ms_MergeSort();
		
		void init(void* data, unsigned int size);
		
		double process();
};

#endif
