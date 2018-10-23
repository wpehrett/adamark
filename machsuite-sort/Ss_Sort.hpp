#include <stdlib.h>

//Required headers for simulator operation
#include "prototypes.hpp"

#define TYPE int32_t
#define TYPE_MAX INT32_MAX

#define SIZE 2048
#define NUMOFBLOCKS 512

#define ELEMENTSPERBLOCK 4
#define RADIXSIZE 4
#define BUCKETSIZE NUMOFBLOCKS*RADIXSIZE
#define MASK 0x3

#define SCAN_BLOCK 16
#define SCAN_RADIX BUCKETSIZE/SCAN_BLOCK

#ifndef SSSORT_HPP
#define SSSORT_HPP

using namespace std;

class Ss_Sort: public Module { //One port, arbitrary data, single-cycle latency, infinite i/o queue sizes
	public:
  	TYPE a[SIZE];
	  TYPE b[SIZE];
	  TYPE bucket[BUCKETSIZE];
	  TYPE sum[SCAN_RADIX];
		//TYPE data[SIZE]; //Memory structure itself
		
		bool running;
		
		Ss_Sort(unsigned int id);
		~Ss_Sort();
		
		void init(void* data, unsigned int size);
		
		double process();
};

#endif
