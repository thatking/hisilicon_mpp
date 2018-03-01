1、使用方法：
	(1) 首先把程序编译成.a库；
	(2) 在应用程序的Makefile添加对tools（主要是能访问tools_shell_cmd.h和libtools.a）的查找路径；编译时，把对libtools.a的依赖放在对SDK库的依赖之前。
	(3) 在应用程序初始化完SDK媒体驱动之后调用tools_cmd_register把tools初始化。
	
2、iq_debug 工具修改记录：
	增加了一个选择参考帧来源的参数 “REF”
	(1) REF = 0时，参考帧来自于重构帧写出；
	(2) REF = 1时，参考帧来自于大码流通道写出；