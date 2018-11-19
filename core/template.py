# <TODO> testcase:
# Input data: <TODO>
# Output data: <TODO>



# tc_name is used as the output program name for the final C++ build
tc_name = '<TODO>'

# Input data file
# data_in must contain a newline-separated list of values of data_in_type
# data_in must have no extraneous lines or information of any sort in the file
# data_in_type must be one of [int32_t, int64_t, double]
data_in = '<TODO>'
data_in_type = '<TODO>'

# Output data file
# data_out must contain a newline-separated list of values of data_out_type
# data_out must have no extraneous lines or information of any sort in the file
# data_out_type must be one of [int32_t, int64_t, double]
data_out = '<TODO>'
data_out_type = '<TODO>'





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
		for i in range(2,len(tokens)): #TODO FIXME make this better-defined, and less potentially-confusing in structure, and also make sure to note in the docs that "init" and "start" cannot appear in the C++ params verbatim! (fine in quotation marks, though, a la string literals)
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
if os.path.exists(tc_name): os.system('rm '+tc_name)
print 'Calling build: '+buildstr
os.system(buildstr)

######## End: build system ########



######## Run it! ########

runstr = './'+tc_name+' -i '+data_in+' -o '+data_out
if args.verbose: runstr += ' -v'
print 'Calling run: '+runstr
os.system(runstr)

######## End: run it! ########
