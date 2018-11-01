/******** DYNAMICALLY GENERATED FILE, DO NOT MODIFY ********/
#include <stdlib.h>
#include <vector>

#include "prototypes.hpp"

#ifndef _SYSCFG
#define _SYSCFG

/******** DYNAMICALLY GENERATED PER SYSCFG FILE, DO NOT MODIFY ********/
#define INPUT_TYPE int32_t
#define OUTPUT_TYPE int32_t
/**********************************************************************/

class SysCfg {
	public:
		std::vector<Module*> modules;
		std::vector<int> mod_init_ids;
		std::vector<int> mod_start_ids;
		
		SysCfg(); //Constructor builds the module list, as auto-generated per configuration file
		~SysCfg();
		
		int cleanList();
};

#endif
/***********************************************************/