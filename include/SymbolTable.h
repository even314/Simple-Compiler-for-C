#ifndef SYMBOTABLE_H
#define SYMBOTABLE_H
#include<iostream>
#include<string>
#include<deque>
#include<stack>
#include<vector>
#include<utility>
#include<map>
#include"utils.h"
using namespace std;

//符号
class Symbol{
public:
    string type="";
    string name="";
    string value="";
    Symbol(){}
    Symbol(string type,string name, string value){
        this->type=type;
        this->name=name;
        this->value=value;
    }
};
class Arr{  //数组
public:
    string type;    //数组类型
    string name;    //数组名
    string len;    //数组长度
    map<string,string>element;  //元素 index-value
    string search_element(string index){
        if(element.find(index)==element.end())return "";
        return element[index];
    }
    Arr(string _type,string _name,string _len){
        type=_type;
        name=_name;
        len=_len;
    }
};

//符号表（一个作用域里）
class SymbolTable
{
public:
    deque<Symbol> table;
    map<string,Arr*>arrays;  //数组列表，名字作键
    Symbol* operator[](string var_name);
   
    int add_var(string _name, string _type,string _value="#0");   //符号表中加标识符
    int add_arr(string _name, string _type,string len);
    SymbolTable(){}
    ~SymbolTable(){}
};
struct Func //函数信息
{
    string type;   // 返回类型
    string name;    //函数名
    vector<Symbol*> para_list;   //参数列表
};

class TableManager{
public:
    vector<SymbolTable*> tables;  // 多个作用域要有多个表;函数作用域互相隔离;最底下是全局变量表
    map<string,Func*> functions; //函数列表，名字做键，规定不能定义同名函数
    map<string,string> constant_pool;  //常量池

    int num_of_temporary=0;
    string gen_tmp_var(string _op, string _arg1="", string _arg2="");   //产生一个临时变量
    Symbol* search_var(string _name,string _type="");    //在所有作用域内搜索变量
    Func* search_func(string func_name);
    Arr* search_array(string arr_name);
    string search_array_element(Arr* arr,string index);
    void update_var(string _name,string _value="",string _type="");    //赋值语句中，更新变量值
    void add_table();
    SymbolTable* pop_table();
};
extern TableManager table_manager;
#endif