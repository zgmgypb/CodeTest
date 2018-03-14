
[应急广播协议处理模块目录说明]

.
├── build.sh		// 编译脚本，直接运行该脚本，直接生成最终需要的模块，生成的模块在 ./lib/release 中，需要不同的编译器编译，修改该脚本中的 PLF 即可;
├── Changelog.txt	// 软件开发和修改日志
├── comm		// 通用函数		
├── demo		// 模块测试代码，使用模块时参考
├── em_broadcast	// 应急广播协议解析代码
├── lib		// 编译中间文件夹，生成的最终文件在 lib/release 中
├── prot_proc	// tcp 接收处理和用于外部调用的接口实现
├── readme.txt	// 本说明
├── tcp_server	// 只测试 TCP 服务器代码
