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

class Ss_Sort: public FixedLatencyModule {
	public:
  	TYPE a[SIZE];
	  TYPE b[SIZE];
	  TYPE bucket[BUCKETSIZE];
	  TYPE sum[SCAN_RADIX];
		
		Ss_Sort(unsigned int id);
		~Ss_Sort();
		
		void init(void* data, unsigned int size);
		
		void process_real(Message* m);
};

#endif
