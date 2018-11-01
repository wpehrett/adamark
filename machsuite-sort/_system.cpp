/******** DYNAMICALLY GENERATED FILE, DO NOT MODIFY ********/
#include <stdlib.h>
#include <vector>
#include <algorithm>

#include "prototypes.hpp"
#include "_system.hpp"

/******** DYNAMICALLY GENERATED PER SYSCFG FILE, DO NOT MODIFY ********/
#include "Ss_Sort.hpp"
/**********************************************************************/

SysCfg::SysCfg() {
/******** DYNAMICALLY GENERATED PER SYSCFG FILE, DO NOT MODIFY ********/
	Ss_Sort* m0 = new Ss_Sort(0);
	modules.push_back(m0);
	mod_init_ids.push_back(0);
	mod_start_ids.push_back(0);
/**********************************************************************/
}

SysCfg::~SysCfg() {
	for (int i=0; i<modules.size(); i++) {
		delete modules[i];
	}
}

int SysCfg::cleanList() {
	//Sort modules vector by ID to improve efficiency of message delivery in sim
	std::sort(modules.begin(), modules.end(), [](const Module* a, const Module* b) {return a->id < b->id;});
	
	//Scan modules vector to ensure all (user-assigned) IDs are unique and strictly sequential; if not, message delivery would break
	for (int i=0; i<modules.size()-1; i++) {
		if ((modules[i]->id)+1 != modules[i+1]->id) return -1;
	}
	
	return 0;
}
/***********************************************************/