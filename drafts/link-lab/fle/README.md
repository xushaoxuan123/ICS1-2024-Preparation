Reference: [可执行文件和加载 by jyy@NJU](https://jyywiki.cn/OS/2024/lect19.md)

> **Funny Little Executable**: 我们 “自行设计” 了能实现 (静态) 链接和加载的二进制文件格式，以及相应的编译器、链接器 (复用 gcc/ld) 和加载器。FLE 文件直接将一段可读、可写、可执行的位置无关代码连通数据映射到内存并跳转执行。

将 python 代码使用 C++ 重写，需要 C++ 23 以上版本

需要安装 vcpkg
