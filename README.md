简单的C语言编译器</br>
系统：linux centos</br>
语言：c++,nasm</br>
目录结构：</br>
    lex.l：词法分析</br>
    parse.y：语法分析</br>
    include：头文件及类：</br>
        GrammarTree.h:抽象语法树</br>
        SymbolTable.h:符号表类</br>
        utils.h:工具类</br>
    source:源文件：</br>
        GrammarTree.cpp:抽象语法树的具体方法,语义分析,生成中间代码及优化</br>
        SymbolTable.cpp:符号表方法</br>
        utils.cpp:工具类方法</br>
    assemble:汇编文件</br>
        functions.asm</br>
        transfer.py:中间代码转汇编</br>
    exec:可执行文件</br>
        build.sh:构造</br>
        其余文件为构造产生的文件</br>
    program:测试案例目录</br>
    results:测试结果目录</br>
依赖：</br>
    gcc,flex,bison</br>
运行方法：</br>
    cd exec</br>
    ./build.sh</br>
