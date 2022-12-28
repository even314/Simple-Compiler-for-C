#include "../include/SymbolTable.h"
#include "../include/utils.h"
#include <iostream>

//重载[]获取符号表中对应名字的元素
Symbol* SymbolTable::operator[](string var_name){
    for (int i = 0; i < table.size(); i++) {
    if (table[i].name == var_name) {
      return &(table[i]);
    }
    }
    return nullptr;
}
// 符号表中添加元素
int SymbolTable::add_var(string _name, string _type,string _value){
    for (auto i : table) {
    if (i.name == _name) { // 变量已经定义过,提示并覆盖原值，并返回1
      print_yellow2(_name+":变量重复定义");
      (*this)[_name]->value = _value;
      (*this)[_name]->type = _type;
      return 1;
    }
  }
  table.push_back(Symbol(_type,_name,_value));
//   cout<<"add_var:("<<_type<<","<<_name<<","<<_value<<")"<<endl;
  return 0;
}
int SymbolTable::add_arr(string _name, string _type,string _len){
    if(arrays.find(_name)!=arrays.end()){
        //已经定义，覆盖定义
        arrays[_name]=new Arr(_type,_name,_len);
        print_yellow2(_name+":数组重复定义");
        return 1;
    }
        arrays[_name]=new Arr(_type,_name,_len);
        return 0;
}
void TableManager::add_table(){
    tables.push_back(new SymbolTable());
}
SymbolTable* TableManager::pop_table(){
    SymbolTable* tmp=*(tables.end()-1);
    tables.pop_back();
    return tmp;
}
Symbol* TableManager::search_var(string _name,string _type){
    // cout<<"search_var:"<<_name<<endl;
    for(auto sybtable=tables.rbegin();sybtable!=tables.rend();sybtable++){ //vector<SymbolTable*> tables
            if((*(*sybtable))[_name]==nullptr)continue; //迭代器取*得SymbolTable*，再取*得SymbolTable
            return (*(*sybtable))[_name];
        }
    return nullptr;
}
//寻找函数
Func* TableManager::search_func(string func_name){
    auto tmp=table_manager.functions.find(func_name);
    if(tmp!=table_manager.functions.end())
        return (*tmp).second;
    return nullptr;   
}
//寻找数组(在所有作用域内)
Arr* TableManager::search_array(string arr_name){ 
    for(auto sybtable=tables.rbegin();sybtable!=tables.rend();sybtable++){ //vector<SymbolTable*> tables
        if((*(*sybtable)).arrays.find(arr_name)==(*(*sybtable)).arrays.end())continue; //迭代器取*得SymbolTable*，再取*得SymbolTable
        return (*(*sybtable)).arrays[arr_name];
    }
    return nullptr; 
}
//寻找某数组元素
string TableManager::search_array_element(Arr* arr,string index){
    string real_index;
    for(auto item:constant_pool)
        if(item.second==index){
            real_index=item.first;
            break;
        }
    if(stoi(real_index)>=stoi(arr->len)){
        //数组越界
        print_red2("数组下标越界:"+arr->name+"["+index+"]");
        exit(-1);
    }
    string val=arr->search_element(index);
    //数组中有这个元素（引用过）
    if(val.size())return val;
    //数组中无这个元素（还没引用过）
    string tmp_var=gen_tmp_var("_");    //用一个临时变量“代表”该元素
    arr->element[index]=tmp_var;    
     for(auto sybtable=tables.rbegin();sybtable!=tables.rend();sybtable++){//临时变量加入符号表
        if((*(*sybtable)).arrays.find(arr->name)==(*(*sybtable)).arrays.end())continue; //迭代器取*得SymbolTable*，再取*得SymbolTable
        (*(*sybtable)).add_var(tmp_var,arr->type);
     }
    // (*(table_manager.tables.end()-1))->add_var(tmp_var,arr->type);  
    return tmp_var;
}
// 赋值语句中，更新变量值，如e=e
void TableManager::update_var(string _name,string _value,string _type){
        Symbol* tmp=search_var(_name,_type);
        if(tmp==nullptr)return;
        if(_value.size())tmp->value=_value;
        if(_type.size())tmp->type=_type;
        // cout<<"update_var:("<<_type<<","<<_name<<","<<_value<<")"<<endl;
}
string TableManager::gen_tmp_var(string _op, string _arg1, string _arg2){
    // 如果是赋值操作,arg1=arg2,更新符号表中的标识符值，要从后到前扫描
    if (_arg2.size() == 0)_arg2 = "0";
    if (_arg1.size() == 0)_arg1 = "0";
    if(_op=="="){
        update_var(_arg1,_arg2);
    }
    num_of_temporary++;
    char buf[32];
    snprintf(buf, 32, "temphh%d", num_of_temporary);
    string tem_var_name(buf);
    // cout<<"gen_tmp_var:"<<tem_var_name<<endl;
    return tem_var_name;
}
TableManager table_manager;