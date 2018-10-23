# Sort testcase:
# Input data: unsorted list of ints
# Output data: sorted list of ints

import argparse
import sys
import subprocess
import os

tc_name = 'sort'
data_in = 'sort-input.data'
data_in_type = 'int32_t'
data_out = 'sort-check.data'
data_out_type = 'int32_t'
# TODO define a comparison function - e.g., for mlperf, a function to check an outputted trained-model for performance within spec
# TODO split out the system definition parser from the testcase definition!?!?

# Parse command-line args
parser = argparse.ArgumentParser()
parser.add_argument('-v', dest='verbose', action='store_const', const=True, default=False)
parser.add_argument('-s', dest='syscfg')
args = parser.parse_args()
if (args.verbose): print 'Verbose flag set'
if not (args.syscfg):
	print 'Error: syscfg not set'
	sys.exit(1)
else: print 'Syscfg: ' + args.syscfg

# Parse system definition file
# Each line must have, in this order, separated by spaces:
#  (1) A unique integer identifier >= 0 for the module in question. Negative values are reserved for the system and must not be used.
#  (2) The case-sensitive name of the module class. This must correspond EXACTLY to the name of the module's top-level .cpp and .hpp files (both are required - the .hpp to include in the system configuration .cpp file we programmatically generate, and the .cpp to include for the actual compilation).
#  (3) A space-separated list, in order, of all parameters (besides module ID) required by the module constructor. These will be passed directly, as-is, to the constructor, and thus must be syntactically correct in C++.
#  (4) (Optional) The keyword "init", which must exist exactly once in the module list. In addition to its process() function, this module must include an init() function following the protoype given in prototypes.hpp (TODO define/operate on this in main.cpp!) which is called to initially ingest and parse the testcase's input data into appropriate user-defined module-internal data structures (e.g., setting up the initial contents of DRAM before simulation begins). (FIXME: This function may not be required for some testcases, e.g. those assuming an input stream of data arriving at periodic intervals.) (FIXME: should it actually need to be only one module?)
#  (5) (Optional) the keyword "start", which, like "init", must exist exactly once in the module list. The testharness will send a Start message to this module at the beginning of simulation. (FIXME: should it actually need to be only one module? Furthermore, is it actually *always* needed in *some* module, or could it just not exist at all and do fine? (answers to both: [probably not] && [the latter]))
# Any module may send a message to the system (destination node id = -1) indicating that computation is complete, containing a properly formatted (per the testcase definition) bundle of data to compare against the reference output.

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
		if init: gens += '\tmod_init = m'+tokens[0]+';\n'
		if start: gens += '\tmod_start_id = '+tokens[0]+';\n'
		sources_cpp.append(tokens[1]+'.cpp')

# Generate _system.cpp
systemcpp_str = ''
systemcpp_str += '''#include <stdlib.h>
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
}'''

# Write _system.cpp
with open('_system.cpp', 'w') as f:
	f.write(systemcpp_str)

# Note: _system.hpp should not be modified!

# Build system
buildstr = 'g++ prototypes.cpp '
for s in sources_cpp:
	buildstr += s+' '
buildstr += '_system.cpp main.cpp -std=c++11 -o '+tc_name
print 'Calling build: '+buildstr
os.system(buildstr)

# Run it!
runstr = './'+tc_name+' -i '+data_in+' -o '+data_out
if args.verbose: runstr += ' -v'
print 'Calling run: '+runstr
os.system(runstr)
