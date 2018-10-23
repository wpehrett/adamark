#include <stdlib.h>
#include <vector>

#include "prototypes.hpp"

#ifndef _SYSCFG
#define _SYSCFG

class SysCfg {
	public:
		std::vector<Module*> modules;
		Module* mod_init;
		int mod_start_id;
		
		SysCfg(); //Constructor builds the module list, as auto-generated per configuration file
		~SysCfg();
		
		int cleanList();
};

#endif
