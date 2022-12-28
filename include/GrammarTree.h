#ifndef GRAMMARTREE_H
#define GRAMMARTREE_H
#include<iostream>
#include<string>
#include<vector>
#include<utility>
#include<queue>
#include<iomanip>
#include<deque>
#include<algorithm>
#include<fstream>
#include "./SymbolTable.h"
using namespace std;

// 中间代码类
class InterCode{
public:
    string op;
    string arg1;
    string arg2;
    string result;

    // 重载输出流运算符
    friend std::ostream& operator<<(ostream& out, InterCode intercode){
        out<<intercode.op<<'\t';
        out<<(intercode.arg1.size()?intercode.arg1:"_")<<'\t';
        out<<(intercode.arg2.size()?intercode.arg2:"_")<<'\t';
        out<<intercode.result;
        return out;
    }
    // 构造
    InterCode(string op,string result,string arg1="",string arg2=""){
        this->op=op;
        this->result=result;
        this->arg1=arg1;
        this->arg2=arg2;
    }
    InterCode(){}
};
// 抽象语法树的节点
class Node{
public:
    string type="";    //类型
    string value="";   //值
    string name="";    //名

    // 中间代码
    vector<pair<int,InterCode>>code;

    // op num
    int num;
    int next;

    //子节点
    deque<Node*>children;

    Node(string type, string args=""){
        this->type=type;
        if(type=="ID")this->name=args;
        else this->value=args;
    }
    void add_child(Node* node){
        children.push_front(node);
    }
};
//抽象语法树
class GrammarTree{
public:
    vector<pair<int,InterCode>>codes;
    int global_code_num = 0; //中间代码序号

    Node* root;
    GrammarTree(){root=nullptr;}

    void print_tree(Node* node, int id);// 打印抽象语法树
    void trim(Node* node);  //剪枝
    void back_patch(Node* node,string mark,string dst); //回填
    void exit_scope(Node* node);// 退出作用域时要把变量的值改回上层作用域
    // void implant_function(Node* node);    //嵌入函数的中间代码段
    string find_constant(Node* node,string num);
    bool check_type(string var1,string var2);   //类型检查
    void analysis(Node* node,int layer);    //语义分析
    void optimize();    //优化
    void gen_intercode(Node *node);     //整合中间代码
    void print_code();   //中间代码打印到文件
};
extern GrammarTree grammar_tree;
#endif