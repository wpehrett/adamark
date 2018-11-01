Welcome to ADAMark!

ADAMark is a set of tools and benchmarks designed to accurately simulate and test massively heterogeneous systems at a functional level, rather than requiring you, the designer, to create formal HDL for everything you want to try. It is intended to remain as user-friendly as possible while placing as few constraints as possible on the design of the hardware to be simulated.

The basic structure is as follows:

	1) A testcase/benchmark is defined in a Python script, <test_name>.py, which specifies the testcase name, the input data and format, and the expected output data and format. It's that simple! There is no predefined algorithm or code or hardware architecture you have to use - all that matters is that your design takes the input data and eventually spits out the correct output data.

	2) A simulated hardware system is built out of a set of functional C++ modules, built using a simple template that extends the abstract Module class in prototypes.hpp. A module has an input queue of Messages (another prototype class) received from other modules, an output queue of Messages to be sent to other modules, and a process() function that defines what computation the module does on a given simulation cycle. Modules also have some inherent parameters, such as a numeric module ID and a latency value (how many cycles it takes to execute the process() function). A module may also have an init() function, which may be called before simulation begins in order to load data into appropriate starting locations in the simulated system (for instance, setting up the initial state of DRAM or cache).

	3) The configuration of the overall system is defined in a <system_name>.syscfg file. This file contains a list of the module IDs, the name of the C++ class containing the module definition, "init" and/or "start" flags to define whether the simulator needs to call that module's init() function or send it a "start" message, respectively, as simulation begins, and any other parameters required for that specific module's constructor.

To run a testcase, use the command "python <test_name>.py" using the following command-line options:
-s <system_name>.syscfg (required) - provide system definition
-v: verbose mode (optional) - increase text output for debugging etc.

----------------

Now for the gory details:

The Python test script parses the system config file, then compiles and runs a simulator binary for that particular system design. Within the system definition file, each line must have, in this order, separated by spaces:

	(1) A unique integer identifier >= 0 for the module in question. Negative values are reserved for the testharness and must not be used.

	(2) The case-sensitive name of the module class. This must correspond EXACTLY to the name of the module's top-level .cpp and .hpp files (both are required - the .hpp to include in the system configuration .cpp file we programmatically generate, and the .cpp to include for the actual compilation).

	(3) A space-separated list, in order, of all parameters (besides module ID) required by the module constructor. These will be passed directly, as-is, to the constructor, and thus must be syntactically correct in C++.

	(4) (Optional) The keyword "init", which should exist at least once in the module list. In addition to the process() function, modules thus denoted must include an init() function following the protoype given in prototypes.hpp, which is called to initially ingest and parse the testcase's input data into appropriate user-defined module-internal data structures (e.g., setting up the initial contents of DRAM before simulation begins).

	(5) (Optional) The keyword "start", which, like "init", should exist at least once in the module list. The testharness will send a Start message to this/these module(s) at the beginning of simulation (see prototypes.hpp).

On each simulation cycle, each Module's process() function is called. In general, the process() function should check for messages in the module's input queue, act accordingly, and then add any needed messages to the module's output queue. Once all process() functions have been executed, the simulator forwards messages from each module's output queue to the destination modules' input queues and then proceeds to the next cycle (or ends, if it receives a Done message from any module).

This message-passing structure inherently approximates a simplistic, fully-connected system in which input/output queue sizes are infinite, messages are transmitted in one cycle and may be of any arbitrary size, etc. More complex, i.e. more realistic, interconnects may be built by wrapping modules in other modules - for instance, by wrapping a NoC interface with proper flow control and limited packet/flit sizes around an accelerator kernel. (Future work intends to permit straightforward interfacing with BookSim, Garnet, etc. as desired.)

The Message class is designed to be as generic as possible in order to permit maximum flexibility to system designers in how they implement inter-module communication. In general, good practice is to set the data field of a Message object to an instance of a well-defined C++ class. The ThReqParams class, for instance (see prototypes.hpp), is designed to enable communication between Modules and the simulator/testharness. This class is used for passing start messages to modules; additionally, any module may send a message to the system (destination node id = -1, ThReqType = Done) indicating that computation is complete and simulation should end. This message must contain a properly structured (per the testcase definition) bundle of data to be compared against the reference output.

For more details, please review the example module, system, and testcase definitions given in Ms_MergeSort.cpp/hpp, sort-merge.syscfg, and sort.py, or contact wpehrett@umich.edu.

Good luck!
