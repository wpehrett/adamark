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
	activeEnergy = 1.2;
	idleEnergy = 0.1;
}
Ms_MergeSort::~Ms_MergeSort() {}

void Ms_MergeSort::init(void* data, unsigned int size) {
	if (data != NULL) {
		//We assume, for this testcase, that the data are an array of [size/sizeof(int32_t)] int32_t's
		for (int i=0; i<(size/sizeof(TYPE)); i++) {
			this->data[i] = ((TYPE*)data)[i];
		}
	}
}

void Ms_MergeSort::process_real(Message* m) {
	ThReqParams* r = (ThReqParams*)(m->data);
	unsigned int sender = m->src;
	//printf("Calling procreal...\n");
	
	if (r->type == Start) { //Start processing data currently in storage
		ms_mergesort(this->data);
	}
	else { //Unknown op, ignore
	}
	
	//Clean up message data objects that were malloc'd or created with C++ "new"
	delete r;
	
	//Send result
	//Since we "know" it goes straight back to the testbench, send as a ThReqParams with Done type
	TYPE* dataOut = (TYPE*)malloc(sizeof(TYPE)*SIZE);
	if (!dataOut) printf("Error in Ms_MergeSort: couldn't allocate output array\n");
	for (int i=0; i<SIZE; i++) {
		dataOut[i] = this->data[i];
	}
	outputs.push_back(new Message((void*)(new ThReqParams(Done, dataOut, SIZE)), this->id, -1));
}
