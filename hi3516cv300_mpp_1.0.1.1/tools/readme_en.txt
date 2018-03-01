1¡¢usage:
	(1) First compile the program into a .a Library; 
	(2) Added the search path of tools(mainly to access tools_shell_cmd.h and libtools.a) in the Makefile of application; When you compile, the dependence on the libtools.a on the SDK library before.
	(3) Calling tools_cmd_register to initialize the tools after the application is initialized SDK.

2¡¢The modification record of iq_debug tool:
	Added an option to select reference frame source parameters "REF";
	(1) When REF = 0, the reference frame is derived from the reconstructed frame;
	(2) When REF = 1, the reference frame is derived from the large stream channel;