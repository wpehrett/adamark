# AES testcase:
# Input data: 32 uint8_t's to use as a key, plus 16 plaintext uint8_t's to be encrypted
# Output data: 16 ciphertext uint8_t's



# tc_name is used as the output program name for the final C++ build
tc_name = 'aes'

# Input data file
# data_in must contain a newline-separated list of values of data_in_type
# data_in must have no extraneous lines or information of any sort in the file
# data_in_type must be one of [int32_t, int64_t, double]
data_in = 'aes-input.data'
data_in_type = 'int32_t'

# Output data file
# data_out must contain a newline-separated list of values of data_out_type
# data_out must have no extraneous lines or information of any sort in the file
# data_out_type must be one of [int32_t, int64_t, double]
data_out = 'aes-check.data'
data_out_type = 'int32_t'





################ DO NOT MODIFY ANYTHING BELOW THIS LINE!!! ################



import argparse
import sys
import subprocess
import os



######## Parse command-line args ########

parser = argparse.ArgumentParser()
parser.add_argument('-v', dest='verbose', action='store_const', const=True, default=False)
parser.add_argument('-s', dest='syscfg')
parser.add_argument
args = parser.parse_args()
if (args.verbose): print 'Verbose flag set'
if not (args.syscfg):
	print 'Error: syscfg not set'
	sys.exit(1)
else: print 'Syscfg: ' + args.syscfg

######## End: parse command-line args ########



######## Parse system definition file ########

# Each line must have, in this order, separated by spaces:
#  (1) A unique integer identifier >= 0 for the module in question. Negative values are reserved for the system and must not be used.
#  (2) The case-sensitive name of the module class. This must correspond EXACTLY to the name of the module's top-level .cpp and .hpp files (both are required - the .hpp to include in the system configuration .cpp file we programmatically generate, and the .cpp to include for the actual compilation).
#  (3) A space-separated list, in order, of all parameters (besides module ID) required by the module constructor. These will be passed directly, as-is, to the constructor, and thus must be syntactically correct in C++.
#  (4) (Optional) The keyword "init", which must exist at least once in the module list. In addition to its process() function, this module must include an init() function following the protoype given in prototypes.hpp, which is called to initially ingest and parse the testcase's input data into appropriate user-defined module-internal data structures (e.g., setting up the initial contents of DRAM before simulation begins). Note that, programmatically, all Module()'s must implement this function, even if the function is empty (i.e., "{}"), to prevent the compiler complaining about pure virtual functions.
#  (5) (Optional) the keyword "start", which, like "init", must exist at least once in the module list. The testharness will send a Start message to this/these module(s) at the beginning of simulation.
# Any module may send a message to the system (destination node id = -1, ThReqType = Done) indicating that computation is complete, containing a properly structured (per the testcase definition) bundle of data to compare against the reference output.

sources_cpp = []
includes = ''
gens = ''
with open(args.syscfg, 'r') as f:
	syscfg_data = f.readlines()
	for l in syscfg_data:
		tokens = l.split()
		params = []
		init = False
		start = False
		for i in range(2,len(tokens)):
			if tokens[i] == 'init': init = True
			elif tokens[i] == 'start': start = True
			else: params.append(tokens[i])
		includes += '#include "'+tokens[1]+'.hpp"\n'
		gens += '\t'+tokens[1]+'* m'+tokens[0]+' = new '+tokens[1]+'('+tokens[0]
		for i in range(0, len(params)):
			gens += ', '+params[i]
		gens += ');\n\tmodules.push_back(m'+tokens[0]+');\n'
		if init: gens += '\tmod_init_ids.push_back('+tokens[0]+');\n'
		if start: gens += '\tmod_start_ids.push_back('+tokens[0]+');\n'
		sources_cpp.append(tokens[1]+'.cpp')

######## End: parse system definition file ########



######## Generate _system.cpp ########

systemcpp_str = ''
systemcpp_str += '''/******** DYNAMICALLY GENERATED FILE, DO NOT MODIFY ********/
#include <stdlib.h>
#include <vector>
#include <algorithm>

#include "prototypes.hpp"
#include "_system.hpp"

/******** DYNAMICALLY GENERATED PER SYSCFG FILE, DO NOT MODIFY ********/
'''
systemcpp_str += includes
systemcpp_str += '''/**********************************************************************/

SysCfg::SysCfg() {
/******** DYNAMICALLY GENERATED PER SYSCFG FILE, DO NOT MODIFY ********/
'''
systemcpp_str += gens
systemcpp_str += '''/**********************************************************************/
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
/***********************************************************/'''

######## End: generate _system.cpp ########



######## Write _system.cpp ########

with open('_system.cpp', 'w') as f:
	f.write(systemcpp_str)

######## End: write _system.cpp ########



######## Generate _system.hpp ########

systemhpp_str = ''
systemhpp_str += '''/******** DYNAMICALLY GENERATED FILE, DO NOT MODIFY ********/
#include <stdlib.h>
#include <vector>

#include "prototypes.hpp"

#ifndef _SYSCFG
#define _SYSCFG

/******** DYNAMICALLY GENERATED PER SYSCFG FILE, DO NOT MODIFY ********/
#define INPUT_TYPE '''
systemhpp_str += data_in_type
systemhpp_str += '''
#define OUTPUT_TYPE '''
systemhpp_str += data_out_type
systemhpp_str += '''
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
/***********************************************************/'''

######## End: generate _system.hpp ########



######## Write _system.hpp ########

with open('_system.hpp', 'w') as f:
	f.write(systemhpp_str)

######## End: write _system.hpp ########



######## Build system ########

buildstr = 'g++ prototypes.cpp '
for s in sources_cpp:
	buildstr += s+' '
buildstr += '_system.cpp main.cpp -std=c++11 -o '+tc_name
print 'Calling build: '+buildstr
os.system(buildstr)

######## End: build system ########



######## Run it! ########

runstr = './'+tc_name+' -i '+data_in+' -o '+data_out
if args.verbose: runstr += ' -v'
print 'Calling run: '+runstr
os.system(runstr)

######## End: run it! ########
