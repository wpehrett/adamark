#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <list>
#include <vector>
#include <algorithm>

//Required headers for simulator operation
#include "prototypes.hpp"

#include "Ms_MergeSort.hpp"

using namespace std;

//Original function from MachSuite
void merge(TYPE a[SIZE], int start, int m, int stop){
    TYPE temp[SIZE];
    int i, j, k;

    for(i=start; i<=m; i++){
        temp[i] = a[i];
    }

    for(j=m+1; j<=stop; j++){
        temp[m+1+stop-j] = a[j];
    }

    i = start;
    j = stop;

    for(k=start; k<=stop; k++){
        TYPE tmp_j = temp[j];
        TYPE tmp_i = temp[i];
        if(tmp_j < tmp_i) {
            a[k] = tmp_j;
            j--;
        } else {
            a[k] = tmp_i;
            i++;
        }
    }
}

//Original function from MachSuite
void ms_mergesort(TYPE a[SIZE]) {
    int start, stop;
    int i, m, from, mid, to;

    start = 0;
    stop = SIZE;

    for(m=1; m<stop-start; m+=m) {
        for(i=start; i<stop; i+=m+m) {
            from = i;
            mid = i+m-1;
            to = i+m+m-1;
            if(to < stop){
                merge(a, from, mid, to);
            }
            else{
                merge(a, from, mid, stop);
            }
        }
    }
}

//Ms_MergeSort constructor/destructor
Ms_MergeSort::Ms_MergeSort(unsigned int id) {
	this->id = id;
	latency = SIZE*ceil(log2(SIZE));
	cycleCount = 0;
	maxPipeline = 1;
	stalled = false;
	//energyPerCycle = 0.0;
	cumulativeEnergy = 0.0;
	running = false;
}
Ms_MergeSort::~Ms_MergeSort() {}

void Ms_MergeSort::init(void* data, unsigned int size) {
	if (data != NULL) {
		//We assume, for this testcase, that the data are an array of [size] int32_t's
		for (int i=0; i<size; i++) {
			this->data[i] = ((TYPE*)data)[i];
		}
	}
}

double Ms_MergeSort::process() {
	if (running) {
		if (cycleCount == latency) { //Done, send output msg
			running = false;
			cycleCount = 0;
			TYPE* dataOut = (TYPE*)malloc(sizeof(TYPE)*SIZE);
			if (!dataOut) printf("Error in Ms_MergeSort: couldn't allocate output array\n");
			for (int i=0; i<SIZE; i++) {
				dataOut[i] = this->data[i];
			}
			outputs.push_back(new Message((void*)(new ThReqParams(Done, dataOut, SIZE)), this->id, -1));
			//return energyPerCycle;
			return 0.0;
		}
		else { //Not done, increment cycle count
			cycleCount++;
			//return energyPerCycle;
			return 0.0;
		}
	}
	else if (inputs.size() > 0) {
		printf("Ms_MergeSort input size: %lu\n", inputs.size());
		printf("Ms_MergeSort output size: %lu\n", outputs.size());
		
		//outputs.push_back(new Message((void*)(new ThReqParams(Done, NULL, 0)), this->id, sender));
		
		//Note: expected input data format is a pointer to a ThReqParams object
		Message* m = inputs.front();
		ThReqParams* r = (ThReqParams*)(m->data);
		unsigned int sender = m->src;
		
		if (r->type == Start) { //Start processing data currently in storage
			cycleCount = 1;
			ms_mergesort(this->data);
			running = true;
		}
		else { //Unknown op, ignore
			//outputs.push_back(new Message((void*)data, this->id, sender));
			//outputs.push_back(new Message((void*)(new ThReqParams(Done, NULL, 0)), this->id, sender));
		}
		delete r;
		delete m;
		inputs.pop_front();
		//return energyPerCycle; //if op, add energy
		return 0.0;
	}
	return 0.0; //if no op, we're treating it as 0-energy for now
}
