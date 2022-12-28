简单的C语言编译器
系统：linux centos
语言：c++,nasm
目录结构：
    lex.l：词法分析
    parse.y：语法分析
    include：头文件及类：
        GrammarTree.h:抽象语法树
        SymbolTable.h:符号表类
        utils.h:工具类
    source:源文件：
        GrammarTree.cpp:抽象语法树的具体方法,语义分析,生成中间代码及优化
        SymbolTable.cpp:符号表方法
        utils.cpp:工具类方法
    assemble:汇编文件
        functions.asm
        transfer.py:中间代码转汇编
    exec:可执行文件
        build.sh:构造
        其余文件为构造产生的文件
    program:测试案例目录
    results:测试结果目录
依赖：
    gcc,flex,bison
运行方法：
    cd exec
    ./build.sh
