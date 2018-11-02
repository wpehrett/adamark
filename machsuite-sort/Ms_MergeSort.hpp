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
/*#define TYPE int64_t
#define TYPE_MAX INT64_MAX*/
/*#define TYPE double
#define TYPE_MAX DOUBLE_MAX*/

#ifndef MSMERGESORT_HPP
#define MSMERGESORT_HPP

using namespace std;

class Ms_MergeSort: public Module {
	public:
		TYPE data[SIZE];
		
		bool running;
		
		Ms_MergeSort(unsigned int id);
		~Ms_MergeSort();
		
		void init(void* data, unsigned int size);
		
		double process();
};

#endif
