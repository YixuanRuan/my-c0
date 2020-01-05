# 项目编译

```shell
mkdir build
cd build
cmake ..
make
```

#使用

```shell
Usage: c0 [options] input 

Positional arguments:
input       	speicify the file to be compiled.

Optional arguments:
-h --help   	show this help message and exit
-t          	perform tokenization for the input file.
-s          	generate assembly code.
-c          	generate binary file.
-o --output 	specify the output file.[Required]
```

#完成功能

基础c0

# 改写

无不等价改写

```
// <C0-program> ::= {<variable-declaration>}{<function-definition>}
// 改写
// <C0-program> ::= <variable-declaration><function-definition>
// <variable-declaration> ::= {<variable-declaration-statement>}
// <function-definition> ::= {<function-definition-statement>}
```

