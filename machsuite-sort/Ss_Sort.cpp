#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <list>
#include <vector>
#include <algorithm>

//Required headers for simulator operation
#include "prototypes.hpp"

#include "Ss_Sort.hpp"

using namespace std;

//Original function from MachSuite
void local_scan(int bucket[BUCKETSIZE])
{
    int radixID, i, bucket_indx;
    for (radixID=0; radixID<SCAN_RADIX; radixID++) {
        for (i=1; i<SCAN_BLOCK; i++){
            bucket_indx = radixID*SCAN_BLOCK + i;
            bucket[bucket_indx] += bucket[bucket_indx-1];
        }
    }
}

//Original function from MachSuite
void sum_scan(int sum[SCAN_RADIX], int bucket[BUCKETSIZE])
{
    int radixID, bucket_indx;
    sum[0] = 0;
    for (radixID=1; radixID<SCAN_RADIX; radixID++) {
        bucket_indx = radixID*SCAN_BLOCK - 1;
        sum[radixID] = sum[radixID-1] + bucket[bucket_indx];
    }
}

//Original function from MachSuite
void last_step_scan(int bucket[BUCKETSIZE], int sum[SCAN_RADIX])
{
    int radixID, i, bucket_indx;
    for (radixID=0; radixID<SCAN_RADIX; radixID++) {
        for (i=0; i<SCAN_BLOCK; i++) {
            bucket_indx = radixID * SCAN_BLOCK + i;
            bucket[bucket_indx] = bucket[bucket_indx] + sum[radixID];
         }
    }
}

//Original function from MachSuite
void init(int bucket[BUCKETSIZE])
{
    int i;
    for (i=0; i<BUCKETSIZE; i++) {
        bucket[i] = 0;
    }
}

//Original function from MachSuite
void hist(int bucket[BUCKETSIZE], int a[SIZE], int exp)
{
    int blockID, i, bucket_indx, a_indx;
    blockID = 0;
    for (blockID=0; blockID<NUMOFBLOCKS; blockID++) {
        for(i=0; i<4; i++) {
            a_indx = blockID * ELEMENTSPERBLOCK + i;
            bucket_indx = ((a[a_indx] >> exp) & 0x3)*NUMOFBLOCKS + blockID + 1;
            bucket[bucket_indx]++;
        }
    }
}

//Original function from MachSuite
void update(int b[SIZE], int bucket[BUCKETSIZE], int a[SIZE], int exp)
{
    int i, blockID, bucket_indx, a_indx;
    blockID = 0;

    for (blockID = 0; blockID < NUMOFBLOCKS; blockID++) {
        for(i=0; i<4; i++) {
            bucket_indx = ((a[blockID * ELEMENTSPERBLOCK + i] >> exp) & 0x3)*NUMOFBLOCKS + blockID;
            a_indx = blockID * ELEMENTSPERBLOCK + i;
            b[bucket[bucket_indx]] = a[a_indx];
            bucket[bucket_indx]++;
        }
    }
}

//Original function from MachSuite
void ss_sort(int a[SIZE], int b[SIZE], int bucket[BUCKETSIZE], int sum[SCAN_RADIX]){
    int exp=0;
    int valid_buffer=0;
    #define BUFFER_A 0
    #define BUFFER_B 1

    for (exp=0; exp<32; exp+=2) {
        init(bucket);
        if (valid_buffer == BUFFER_A) {
            hist(bucket, a, exp);
        } else {
            hist(bucket, b, exp);
        }

        local_scan(bucket);
        sum_scan(sum, bucket);
        last_step_scan(bucket, sum);

        if (valid_buffer==BUFFER_A) {
            update(b, bucket, a, exp);
            valid_buffer = BUFFER_B;
        } else {
            update(a, bucket, b, exp);
            valid_buffer = BUFFER_A;
        }
    }
    // If trip count is even, buffer A will be valid at the end.
}

//Ss_Sort constructor/destructor
Ss_Sort::Ss_Sort(unsigned int id) {
	this->id = id;
	latency = RADIXSIZE*SIZE; //FIXME to the *actual* latency of the algorithm
	cycleCount = 0;
	maxPipeline = 1;
	stalled = false;
	cumulativeEnergy = 0.0;
	running = false;
}
Ss_Sort::~Ss_Sort() {}

void Ss_Sort::init(void* data, unsigned int size) {
	if (data != NULL) {
		//We assume, for this testcase, that the data are an array of [size/sizeof(int32_t)] int32_t's
		for (int i=0; i<(size/sizeof(TYPE)); i++) {
			this->a[i] = ((TYPE*)data)[i];
			this->b[i] = ((TYPE*)data)[i];
		}
	}
}

double Ss_Sort::process() {
	if (running) {
		if (cycleCount == latency) { //Done, send output msg
			running = false;
			cycleCount = 0;
			TYPE* dataOut = (TYPE*)malloc(sizeof(TYPE)*SIZE);
			if (!dataOut) printf("Error in Ss_Sort: couldn't allocate output array\n");
			for (int i=0; i<SIZE; i++) {
				dataOut[i] = this->a[i]; //WARNING: this depends on the radix chosen, as noted in the original source (above)!
			}
			outputs.push_back(new Message((void*)(new ThReqParams(Done, dataOut, SIZE)), this->id, -1));
			return 0.0; //FIXME change to energy value, if applicable
		}
		else { //Not done, increment cycle count
			cycleCount++;
			return 0.0; //FIXME change to energy value, if applicable
		}
	}
	else if (inputs.size() > 0) {
		//Note: expected input data format is a pointer to a ThReqParams object
		Message* m = inputs.front();
		ThReqParams* r = (ThReqParams*)(m->data);
		unsigned int sender = m->src;
		
		if (r->type == Start) { //Start processing data currently in storage
			cycleCount = 1;
			ss_sort(this->a, this->b, this->bucket, this->sum);
			running = true;
		}
		else { //Unknown op, ignore
		}
		
		//Clean up message data objects that were malloc'd or created with C++ "new"
		delete r;
		delete m;
		inputs.pop_front();
		
		return 0.0; //FIXME change to energy value, if applicable
	}
	return 0.0; //if no op, we're treating it as 0-energy for now
}
