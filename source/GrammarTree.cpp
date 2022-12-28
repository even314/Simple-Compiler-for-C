#include "../include/GrammarTree.h"
#define PUSH_CODE(num,intercode)node->code.push_back(pair<int,InterCode>(num,intercode));
#define DEFAULT_ACTION for(auto child:node->children)analysis(child,-1);    
#define CHECK_POINT(node)  cout<<"analysis:"<<node->type<<"&"<<node->value<<"&"<<layer<<endl;
GrammarTree grammar_tree;

//打印抽象语法树
void GrammarTree::print_tree(Node *node, int indent) {
  for (int i = 0; i < indent; i++) cout << "  ";
  if (node->type.size()) 
    cout << node->type << "    " << node->name << "" << node->value << endl;
    for (auto i : node->children)print_tree(i, indent + 1);
}
//剪枝，将多语句放进一个队列中
void GrammarTree::trim(Node* node){
    for (int i = 0; i < node->children.size(); i++) {
    if (node->children[i]->type == "s_list" ||
        node->children[i]->type == "Did_list"||
        node->children[i]->type == "dec_param_list" ||
        node->children[i]->type == "param_list") {
      Node *tmp = node->children[i];
      if (tmp->children.size() > 1) {
        queue<Node *> result;
        while (tmp->children.size() > 1) {
          result.push(tmp->children[0]);
          tmp = tmp->children[1];
        }
        result.push(tmp->children[0]);
        node->children[i]->children.clear();
        while (result.size()) {
          node->children[i]->children.push_back(result.front());
          result.pop();
        }
      }
      trim(node->children[i]);
    } else {
      trim(node->children[i]);
    }
  }
}  
//回填
void GrammarTree::back_patch(Node* node,string mark,string dst){
    for(int i=0;i<node->code.size();i++)
        if(node->code[i].second.result==mark)node->code[i].second.result=dst;
    for (auto i : node->children)back_patch(i, mark,dst);
} 
// 退出作用域时要把变量的值改回上层作用域的
void GrammarTree::exit_scope(Node* node){
    SymbolTable* tmp=table_manager.pop_table();
    for(auto var:tmp->table){
        Symbol* var_last=table_manager.search_var(var.name); //改变量在上一个作用域中的位置
        if(var_last!=nullptr)PUSH_CODE(global_code_num++,InterCode("=",var.name,var_last->value))
    }
    // cout<<node->type<<" exit scope"<<endl;
}   
string GrammarTree::find_constant(Node* node,string num){
    auto tmp=table_manager.constant_pool.find(num); 
    if(tmp!=table_manager.constant_pool.end())return(*tmp).second;
    else{
        string tmp_var=table_manager.gen_tmp_var("_");
        PUSH_CODE(global_code_num++,InterCode("=",tmp_var,"#"+num))
        table_manager.constant_pool[num]=tmp_var;
        return tmp_var;
    }
}
bool GrammarTree::check_type(string var1,string var2=""){  //类型检查
    auto var_ptr1=table_manager.search_var(var1);
    if(var2.size()!=0){
        auto var_ptr2=table_manager.search_var(var2);
        if(var_ptr1!=nullptr&&var_ptr2!=nullptr){
            if(var_ptr1->type!=var_ptr2->type)return false;
            if (var_ptr1->type!="int"||var_ptr2->type!="int")return false;
        }
    }
    if(var_ptr1!=nullptr)
        if(var_ptr1->type!="int")
            return false;
    return true;
}   
//语义分析,生成中间代码
void GrammarTree::analysis(Node* node,int layer){
    if(node->type=="all"){  //all->[p,p,s,s,..]
        // CHECK_POINT(node)
        table_manager.add_table();  //新建全局作用域（tables中的第一个表） 
        for(auto child:node->children)
            analysis(child,-1); 
    }else if(node->type=="p"){  //函数定义,规定不能定义同名函数 p->T ID dec_param_list s_list|p->T ID s_list
        table_manager.add_table();  //新建函数作用域
        string return_type=node->children[0]->value;   //返回类型
        string func_name=node->children[1]->name;   //函数名
        if(table_manager.search_func(func_name)!=nullptr){
            //函数已定义过，报错->中断执行
        print_red2(func_name+":函数已定义！");
        table_manager.pop_table();  //销毁作用域
        table_manager.constant_pool.clear();    //清除常量池
        exit(-1);
        }
        else{
            PUSH_CODE(global_code_num++, InterCode("FUNCTION", func_name))    //函数声明
            if(node->children.size()==4){   //有参函数
            vector<Symbol*> param_list;
            //取参数列表
            for(auto dec_param:node->children[2]->children){  //dec_param_list->dec_param,dec_param 
                string param_type=dec_param->children[0]->value;   //dec_param->T ID  规定不带缺省值
                string param_name=dec_param->children[1]->name;
                param_list.push_back(new Symbol(param_type,param_name,""));
                (*(table_manager.tables.end()-1))->add_var(param_name,param_type);  //加进符号表中
                PUSH_CODE(global_code_num++, InterCode("param",param_name)) //规定类型只能是int,不检查类型
            }
            table_manager.functions[func_name]=new Func{return_type,func_name,param_list};  //加进函数表中
            //继续分析，生成中间代码
            analysis(node->children[3],-1); //s_list
        }
        else if(node->children.size()==3){  //无参函数
            table_manager.functions[func_name]=new Func{return_type,func_name};
            //继续分析，生中间代码
            analysis(node->children[2], -1);    //s_list
        }
        table_manager.pop_table();  //销毁作用域
        table_manager.constant_pool.clear();    //清除常量池
        }
        }
    else if(node->type=="e"){   //expression
        // CHECK_POINT(node)
        if(node->children.size()==1){   //e->ID|NUM|STR
        // for(auto child:node->children)CHECK_POINT(child)
            if(node->children[0]->type=="ID"){
                if(table_manager.search_var(node->children[0]->name)==nullptr)  //变量未定义
                {
                print_red2( node->children[0]->name+":变量未定义");
                exit(-1);
                }
                node->value=node->children[0]->name; //头节点的值替换  
            }
            else if (node->children[0]->type=="NUM"){
                string num=node->children[0]->value;
                node->value=find_constant(node,num);
            }
            else if (node->children[0]->type == "STR") 
                node->value = node->children[0]->value;
        if(layer==2){  //布尔式
        PUSH_CODE(global_code_num++, InterCode("Jnz", "e.true", node->value))
        PUSH_CODE(global_code_num++, InterCode("GOTO", "e.false"))
        }
        }
        else if(node->children.size()==2){  //e->e INC(DEC)|INC(DEC) e|!e|-e|T Didlist
            // for(auto child:node->children)CHECK_POINT(child)
            if(layer==-1){  //非布尔式，只能是自增减式和负号，规定！只能在条件判断中用
                if(node->children[0]->type=="INC"){   //++e e=e+1
                    analysis(node->children[1],-1);
                    if(!check_type(node->children[1]->value)){
                        print_red2(node->children[1]->value+":类型错误,不能进行运算!");
                        exit(-1);
                    }
                    string tmp_var=table_manager.gen_tmp_var("+");
                    string one=find_constant(node,"1");
                    PUSH_CODE(global_code_num++,InterCode("+",tmp_var,node->children[1]->value,one))
                    PUSH_CODE(global_code_num++,InterCode("=",node->children[1]->value,tmp_var))
                    //PUSH_CODE(global_code_num++,InterCode("INC",node->children[1]->value))
                    table_manager.update_var(node->children[1]->value,tmp_var);
                    node->value=node->children[1]->value;
                }else if(node->children[0]->type=="DEC"){   //--e
                    analysis(node->children[1],-1);
                    if(!check_type(node->children[1]->value)){
                        print_red2(node->children[1]->value+":类型错误,不能进行运算!");
                        exit(-1);
                    }                    
                    string tmp_var=table_manager.gen_tmp_var("-");
                    string one=find_constant(node,"1");
                    PUSH_CODE(global_code_num++,InterCode("-",tmp_var,node->children[1]->value,one))
                    PUSH_CODE(global_code_num++,InterCode("=",node->children[1]->value,tmp_var))
                    //PUSH_CODE(global_code_num++,InterCode("DEC",node->children[1]->value))
                    table_manager.update_var(node->children[1]->value,tmp_var);
                    node->value=node->children[1]->value;
                }else if(node->children[1]->type=="INC"){   //e++
                    analysis(node->children[0],-1);
                    if(!check_type(node->children[0]->value)){
                        print_red2(node->children[0]->value+":类型错误,不能进行运算!");
                        exit(-1);
                    }                    
                    string tmp_var_old=table_manager.gen_tmp_var("_");
                    PUSH_CODE(global_code_num++,InterCode("=",tmp_var_old,node->children[0]->value))
                    node->value=tmp_var_old;
                    string tmp_var_new=table_manager.gen_tmp_var("+");
                    string one=find_constant(node,"1");
                    PUSH_CODE(global_code_num++,InterCode("+",tmp_var_new,node->children[0]->value,one))
                    PUSH_CODE(global_code_num++,InterCode("=",node->children[0]->value,tmp_var_new))
                    // PUSH_CODE(global_code_num++,InterCode("INC",node->children[0]->value))
                    table_manager.update_var(node->children[1]->value,tmp_var_new);
                }else if(node->children[1]->type=="DEC"){   //e--
                    analysis(node->children[0],-1);
                    if(!check_type(node->children[0]->value)){
                        print_red2(node->children[0]->value+":类型错误,不能进行运算!");
                        exit(-1);
                    }  
                    string tmp_var_old=table_manager.gen_tmp_var("_");
                    PUSH_CODE(global_code_num++,InterCode("=",tmp_var_old,node->children[0]->value))
                    node->value=tmp_var_old;
                    string tmp_var_new=table_manager.gen_tmp_var("-");
                    string one=find_constant(node,"1");
                    PUSH_CODE(global_code_num++,InterCode("-",tmp_var_new,node->children[0]->value,one))
                    PUSH_CODE(global_code_num++,InterCode("=",node->children[0]->value,tmp_var_new))
                    //PUSH_CODE(global_code_num++,InterCode("DEC",node->children[0]->value))
                    table_manager.update_var(node->children[1]->value,tmp_var_new);
                }else if(node->children[0]->type=="-"){ //e->-e  a=0-a
                    analysis(node->children[1],-1);
                    if(!check_type(node->children[1]->value)){
                        print_red2(node->children[1]->value+":类型错误,不能进行运算!");
                        exit(-1);
                    }  
                    string tmp_var=table_manager.gen_tmp_var("-");
                    auto iter=table_manager.constant_pool.find("0");
                    string zero=find_constant(node,"0");
                    PUSH_CODE(global_code_num++,InterCode("-",tmp_var,zero,node->children[1]->value))
                    // PUSH_CODE(global_code_num++,InterCode("=",node->children[1]->value,tmp_var))
                    // table_manager.update_var(node->children[1]->value,tmp_var);
                    node->value=tmp_var;
                }else if(node->children[0]->type=="T"&&node->children[1]->type=="Did_list"){    //e->T Dlist 不处理node->value，变量声明不能作为分式参与运算（可能需要错误处理）
                    Node* Did_list=node->children[1];   //取出变量列表 Dlist.children=[Did,Did,...]
                    for(auto Did:Did_list->children){  //Did->ID|ID ='e'
                        if(Did->children.size()==3){
                            analysis(Did->children[2],-1);  //e
                            (*(table_manager.tables.end()-1))->add_var(Did->children[0]->name,node->children[0]->value,
                            Did->children[2]->value);   //加进符号表，符号表只有标记的作用
                            PUSH_CODE(global_code_num++,InterCode("=",Did->children[0]->name,Did->children[2]->value))  //赋值
                        }else if(Did->children.size()==1){
                            (*(table_manager.tables.end()-1))->add_var(Did->children[0]->name,node->children[0]->value); 
                    }
                    }
            }
            }
            else if(layer==2){  //布尔式
                if(node->children[0]->type=="INC"){   //++e e=e+1
                    analysis(node->children[1],-1);
                    if(!check_type(node->children[1]->value)){
                        print_red2(node->children[1]->value+":类型错误,不能进行运算!");
                        exit(-1);
                    }  
                    string tmp_var=table_manager.gen_tmp_var("+");
                    string one=find_constant(node,"1");
                    PUSH_CODE(global_code_num++,InterCode("+",tmp_var,node->children[1]->value,one))
                    PUSH_CODE(global_code_num++,InterCode("=",node->children[1]->value,tmp_var))
                    table_manager.update_var(node->children[1]->value,tmp_var);
                    node->value=node->children[1]->value;
                    PUSH_CODE(global_code_num++,InterCode("Jnz","e.true",tmp_var))
                    PUSH_CODE(global_code_num++,InterCode("GOTO","e.false"))
                }else if(node->children[0]->type=="DEC"){   //--e
                    analysis(node->children[1],-1);
                    if(!check_type(node->children[1]->value)){
                        print_red2(node->children[1]->value+":类型错误,不能进行运算!");
                        exit(-1);
                    }                      
                    string tmp_var=table_manager.gen_tmp_var("-");
                    string one=find_constant(node,"1");
                    PUSH_CODE(global_code_num++,InterCode("-",tmp_var,node->children[1]->value,one))
                    PUSH_CODE(global_code_num++,InterCode("=",node->children[1]->value,tmp_var))
                    table_manager.update_var(node->children[1]->value,tmp_var);
                    node->value=node->children[1]->value;
                    PUSH_CODE(global_code_num++,InterCode("Jnz","e.true",tmp_var))
                    PUSH_CODE(global_code_num++,InterCode("GOTO","e.false"))
                }else if(node->children[1]->type=="INC"){   //e++
                    analysis(node->children[0],-1);
                    if(!check_type(node->children[0]->value)){
                        print_red2(node->children[0]->value+":类型错误,不能进行运算!");
                        exit(-1);
                    }                      
                    string tmp_var_old=table_manager.gen_tmp_var("_");
                    PUSH_CODE(global_code_num++,InterCode("=",tmp_var_old,node->children[0]->value))
                    node->value=tmp_var_old;
                    string tmp_var_new=table_manager.gen_tmp_var("+");
                    string one=find_constant(node,"1");
                    PUSH_CODE(global_code_num++,InterCode("+",tmp_var_new,node->children[0]->value,one))
                    PUSH_CODE(global_code_num++,InterCode("=",node->children[0]->value,tmp_var_new))
                    PUSH_CODE(global_code_num++,InterCode("Jnz","e.true",tmp_var_old))
                    PUSH_CODE(global_code_num++,InterCode("GOTO","e.false"))
                }else if(node->children[1]->type=="DEC"){   //e--
                    analysis(node->children[0],-1);
                    if(!check_type(node->children[0]->value)){
                        print_red2(node->children[0]->value+":类型错误,不能进行运算!");
                        exit(-1);
                    }                      
                    string tmp_var_old=table_manager.gen_tmp_var("_");
                    PUSH_CODE(global_code_num++,InterCode("=",tmp_var_old,node->children[0]->value))
                    node->value=tmp_var_old;
                    string tmp_var_new=table_manager.gen_tmp_var("-");
                    string one=find_constant(node,"1");
                    PUSH_CODE(global_code_num++,InterCode("-",tmp_var_new,node->children[0]->value,one))
                    PUSH_CODE(global_code_num++,InterCode("=",node->children[0]->value,tmp_var_new))
                    PUSH_CODE(global_code_num++,InterCode("Jnz","e.true",tmp_var_old))
                    PUSH_CODE(global_code_num++,InterCode("GOTO","e.false"))
                }else if(node->children[0]->type=="!"){ //e->!e
                    // CHECK_POINT(node)
                    // for(auto i:node->children[1]->children)cout<<"233:"<<i->type<<endl;
                    analysis(node->children[1],2);
                    back_patch(node->children[1],"e.true","tmp");
                    back_patch(node->children[1],"e.false","e.true");
                    back_patch(node->children[1],"tmp","e.false");
                }else if(node->children[0]->type=="-"){ //只需把e当作布尔式处理，其他跟非布尔式处理一样
                    analysis(node->children[1],2);
                    if(!check_type(node->children[1]->value)){
                        print_red2(node->children[1]->value+":类型错误,不能进行运算!");
                        exit(-1);
                    }                     
                    string tmp_var=table_manager.gen_tmp_var("-");
                    string zero=find_constant(node,"0");
                    PUSH_CODE(global_code_num++,InterCode("-",tmp_var,zero,node->children[1]->value))
                    PUSH_CODE(global_code_num++,InterCode("=",node->children[1]->value,tmp_var))
                    table_manager.update_var(node->children[1]->value,tmp_var);
                    node->value=node->children[1]->value;
                }else DEFAULT_ACTION
            }
        }
        else if(node->children.size()==3){  // 三元运算、赋值语句、调用无参函数
        //    for(auto child:node->children)CHECK_POINT(child)
           if(layer==-1){  // 非布尔式
                if(node->children[1]->type=="="){   //e->e=e
                    analysis(node->children[2],-1);
                    analysis(node->children[0],-1);
                    if(!check_type(node->children[2]->value,node->children[0]->value)){
                        print_red2(node->children[0]->value+','+node->children[2]->value+":类型错误,不能进行运算!");
                        exit(-1);
                    }                                           
                    table_manager.update_var(node->children[0]->value,node->children[2]->value);
                    PUSH_CODE(global_code_num++,InterCode("=",node->children[0]->value,node->children[2]->value))
                }
                else if(node->children[1]->type == "+" ||
                   node->children[1]->type == "-" ||
                   node->children[1]->type == "*" ||
                   node->children[1]->type == "/" ||
                   node->children[1]->type == "%" ||
                   node->children[1]->type == "^" ||
                   node->children[1]->type == "<" ||
                   node->children[1]->type == "LE" ||
                   node->children[1]->type == ">" ||
                   node->children[1]->type == "GE" ||
                   node->children[1]->type == "EQ" ||
                   node->children[1]->type == "NE" ||
                   node->children[1]->type == "AND" ||
                   node->children[1]->type == "OR"){    //e->e+e
                    analysis(node->children[0], -1);
                    analysis(node->children[2], -1);
                    if(!check_type(node->children[0]->value,node->children[2]->value)){
                        print_red2(node->children[0]->value+','+node->children[2]->value+":类型错误,不能进行运算!");
                        exit(-1);
                    }                       
                    string tmp_var=table_manager.gen_tmp_var(node->children[1]->type);
                    PUSH_CODE(global_code_num++,InterCode(node->children[1]->type,tmp_var,node->children[0]->value,node->children[2]->value))
                    node->value=tmp_var;
                   }else if(node->children[0]->type=="INPUT"){  //e->INPUT()
                        string tmp_var=table_manager.gen_tmp_var("CALL","INPUT");
                        PUSH_CODE(global_code_num++,InterCode("CALL",tmp_var,"INPUT"))
                        node->value=tmp_var;
                   }else if(node->children[0]->type=="("&&node->children[2]->type==")"){  //e->(e)
                        analysis(node->children[1],-1); 
                        node->value=node->children[1]->value;  
                   }else if((node->children[0]->type=="ID")&&(node->children[1]->type=="(")&&(node->children[2]->type==")")){
                        //调用无参函数
                        string func_name=node->children[0]->name;    //函数名
                        Func* func=table_manager.search_func(func_name);
                        if(func==nullptr){
                            //函数未定义，报错  （然后中断执行）
                            print_red2(func_name +":函数未定义!");
                            exit(-1);
                        }
                        else{
                            if(func->para_list.size()>0){
                            //参数不匹配，报错  （然后中断执行）
                            print_red2("参数个数不匹配!");
                            exit(-1);
                            }else{
                                //生成中间代码：返回值赋到临时变量
                                string temp_var = table_manager.gen_tmp_var( "CALL", func_name);
                                PUSH_CODE(global_code_num++,InterCode("CALL", temp_var,func_name))
                                node->value = temp_var;
                            }
                        }
                   }
                   else DEFAULT_ACTION
            }else if(layer==2){//布尔式
            if(node->children[1]->type=="="){// if(t1=t2) 布尔结果看t2的值
                analysis(node->children[2],-1);
                analysis(node->children[0],-1);
                if(!check_type(node->children[2]->value,node->children[0]->value)){
                    print_red2(node->children[0]->value+','+node->children[2]->value+":类型错误,不能进行运算!");
                    exit(-1);
                }                   
                table_manager.update_var(node->children[0]->value,node->children[2]->value);
                PUSH_CODE(global_code_num++,InterCode("=",node->children[0]->value,node->children[2]->value))
                PUSH_CODE(global_code_num++,InterCode("Jnz","e.ture",node->children[2]->value))
                PUSH_CODE(global_code_num++,InterCode("GOTO","e.false"));
            }else if(node->children[1]->type == "+" ||
                   node->children[1]->type == "-" ||
                   node->children[1]->type == "*" ||
                   node->children[1]->type == "/" ||
                   node->children[1]->type == "%" ||
                   node->children[1]->type == "^"){ //算术运算 if(t1+t2)1.存值 2.布尔拉链：运算结果与0比较
                analysis(node->children[0],-1);
                analysis(node->children[2],-1);
                if(!check_type(node->children[0]->value,node->children[2]->value)){
                        print_red2(node->children[0]->value+','+node->children[2]->value+":类型错误,不能进行运算!");
                        exit(-1);
                    }  
                string tmp_var=table_manager.gen_tmp_var(node->children[1]->type);
                PUSH_CODE(global_code_num++,InterCode(node->children[1]->type,tmp_var,node->children[0]->value,node->children[2]->value))
                node->value=tmp_var;
                PUSH_CODE(global_code_num++,InterCode("Jnz","e.true",tmp_var))
                PUSH_CODE(global_code_num++,InterCode("GOTO","e.false"))
                   }else if(node->children[1]->type == "<" ||
                   node->children[1]->type == "LE" ||
                   node->children[1]->type == ">" ||
                   node->children[1]->type == "GE" ||
                   node->children[1]->type == "EQ" ||
                   node->children[1]->type == "NE"){  //比较运算 if(t1>t2) 布尔拉链：直接判断比较结果的真假
                   analysis(node->children[0],-1);
                   analysis(node->children[2],-1);
                   if(!check_type(node->children[0]->value,node->children[2]->value)){
                        print_red2(node->children[0]->value+','+node->children[2]->value+":类型错误,不能进行运算!");
                        exit(-1);
                    }  
                   PUSH_CODE(global_code_num++,InterCode(string()+"J"+node->children[1]->type,
                   "e.true",node->children[0]->value,node->children[2]->value))
                   PUSH_CODE(global_code_num++,InterCode("GOTO","e.false"))
                   }else if(node->children[1]->type=="AND"){    //逻辑运算if(t1 AND t2)的布尔拉链：左右两式的拉链逻辑运算
                   analysis(node->children[0],2);   //左子式拉链
                   stringstream ss;
                   string tmp;
                   ss<<global_code_num;
                   ss>>tmp;
                   back_patch(node->children[0],"e.true",tmp);    //左子式回填
                   analysis(node->children[2],2);
                   }else if(node->children[1]->type=="OR"){
                   analysis(node->children[0],2);
                   stringstream ss;
                   string tmp;
                   ss<<global_code_num;
                   ss>>tmp;
                   back_patch(node->children[0],"e.false",tmp);    
                   analysis(node->children[2],2);
                   }else if(node->children[1]->type=="INPUT"){
                        string tmp_var=table_manager.gen_tmp_var("CALL","INPUT");
                        PUSH_CODE(global_code_num++,InterCode("Jnz","e.true",tmp_var))
                        PUSH_CODE(global_code_num++, InterCode("GOTO", "e.false"))
                        node->value=tmp_var;
                   }else if(node->children[0]->type=="("&&node->children[2]->type==")"){    //  括号表达式
                    //把括号去掉，替换，直接指向子节点
                    node=node->children[1];
                    analysis(node,2);
                   }else if((node->children[0]->type=="ID")&&(node->children[1]->type=="(")&&(node->children[2]->type==")")){
                        //调用无参函数  布尔式判断return的值
                        string func_name=node->children[0]->name;    //函数名
                        Func* func=table_manager.search_func(func_name);
                        if(func==nullptr){
                            //函数未定义，报错  （然后中断执行）
                            print_red2(func_name +":函数未定义!");
                            exit(-1);
                        }
                        else{
                            if(func->para_list.size()>0){
                            //参数不匹配，报错  （然后中断执行）
                             print_red2("参数不匹配!");
                             exit(-1);
                            }else{
                                //生成中间代码
                                string tmp_var = table_manager.gen_tmp_var( "CALL", node->children[0]->name);
                                PUSH_CODE(global_code_num++,InterCode("CALL", tmp_var, node->children[0]->name))
                                node->value = tmp_var;
                                PUSH_CODE(global_code_num++,InterCode("Jnz","e.true",tmp_var))
                                PUSH_CODE(global_code_num++,InterCode("GOTO","e.false"))
                            }
                        }
                   }else DEFAULT_ACTION
        }
        }else if(node->children.size()==4){   //调用有参函数或引用数组  e->ID(param_list)|ID[e]
            // for(auto child:node->children)CHECK_POINT(child)
            if(node->children[1]->type=="("&&node->children[3]->type==")"){
            string func_name=node->children[0]->name;    //函数名
            Func* func=table_manager.search_func(func_name);
            if(func==nullptr){
                //函数未定义，报错  （然后中断执行）
                 print_red2(func_name +":函数未定义!");
                 exit(-1);
            }
            else{
                vector<string> param_value; //传入的参数值
                for(auto param:node->children[2]->children){    //param:[e,e,e,e]
                    analysis(param,-1);
                    param_value.push_back(param->value);    
                }
                if(func->para_list.size()!=param_value.size()){
                //参数不匹配，报错  （然后中断执行）
                    print_red2("参数不匹配!");
                    exit(-1);
                }else{
                    //生成中间代码
                    for(auto param:param_value)    //参数代入
                         PUSH_CODE(global_code_num++, InterCode("ARG", "_", param))
                    string tmp_var=table_manager.gen_tmp_var("CALL");
                    PUSH_CODE(global_code_num++,InterCode("CALL",tmp_var,func_name))
                    node->value=tmp_var;
                    if(layer==2){
                         PUSH_CODE(global_code_num++,InterCode("Jnz","e.true",tmp_var))
                         PUSH_CODE(global_code_num++,InterCode("GOTO","e.false"))
                    }
                }
            }
            }else if(node->children[1]->type=="["&&node->children[3]->type=="]"){   //ID[e]
                    string arr_name=node->children[0]->name;
                    Arr* arr=table_manager.search_array(arr_name);
                    if(arr==nullptr){
                        print_red2(arr_name+":数组未定义");
                        exit(-1);   //中断执行
                    }
                    analysis(node->children[2],-1);
                    string index=node->children[2]->value;
                    node->value=table_manager.search_array_element(arr,index);
            }else DEFAULT_ACTION
        }
    }else if(node->type=="s"){  //要考虑作用域问题
        if(node->children[0]->type=="IF"){  //s.children:(子节点)if e s_list
            if(node->children.size()==3){   //s->IF e s_list
                analysis(node->children[1],2);  //e
                stringstream ss;
                string tmp;
                ss << global_code_num;
                ss>>tmp;
                back_patch(node->children[1],"e.true",tmp);
                table_manager.add_table();  //创建作用域
                analysis(node->children[2],0);  //s_list
                exit_scope(node);   //s_list之后就要退出作用域，然后再将e.false回填（执行下一条）
                ss.clear();
                ss<<global_code_num;
                ss>>tmp;
                back_patch(node->children[1],"e.false",tmp);
            }else if(node->children.size()==5){ //if(e)s_list1 else s_list2
                analysis(node->children[1],2);  //e
                stringstream ss;
                string tmp;
                ss<<global_code_num;
                ss>>tmp;
                back_patch(node->children[1],"e.true",tmp);
                ss.clear();
                table_manager.add_table();  //新建作用域
                analysis(node->children[2],0); //s_list1
                exit_scope(node);   //退出作用域
                PUSH_CODE(global_code_num++,InterCode("GOTO","s.next"))
                ss<<global_code_num;
                ss>>tmp;
                ss.clear();
                back_patch(node->children[1],"e.false",tmp);
                table_manager.add_table();  //新建作用域
                analysis(node->children[4],0); //s_list2
                exit_scope(node);   //退出作用域
                ss<<global_code_num;
                ss>>tmp;
                back_patch(node,"s.next",tmp);
            }else DEFAULT_ACTION
        }else if(node->children[0]->type=="WHILE"){ //while(e){s_list}
                stringstream ss;
                string while_begin;
                ss<<global_code_num;
                ss>>while_begin;
                analysis(node->children[1],2);  //e
                string tmp;
                ss.clear();
                ss<<global_code_num;
                ss>>tmp;
                back_patch(node->children[1],"e.true",tmp);
                table_manager.add_table();  //新建作用域
                analysis(node->children[2],0); //s_list
                PUSH_CODE(global_code_num++,InterCode("GOTO",while_begin))  //回到判断条件
                ss.clear();
                ss<<global_code_num;
                ss>>tmp;             
                back_patch(node->children[1],"e.false",tmp);    //条件不满足，跳出while
                exit_scope(node);   //退出作用域   
        }else if(node->children[0]->type=="FOR"){   //for(e1,e2,e3){s_list}
                table_manager.add_table();  //新建作用域
                analysis(node->children[1],-1); //e1    
                stringstream ss;
                string for_begin;
                ss<<global_code_num;
                ss>>for_begin;
                ss.clear();
                analysis(node->children[2],2);  //e2,条件判断的布尔式
                string tmp;
                ss<<global_code_num;
                ss>>tmp;
                ss.clear();
                back_patch(node->children[2],"e.true",tmp);
                analysis(node->children[4],0); //s_list
                analysis(node->children[3],-1); //e3
                PUSH_CODE(global_code_num++,InterCode("GOTO",for_begin))
                ss<<global_code_num;
                ss>>tmp;    
                back_patch(node->children[2],"e.false",tmp);
                exit_scope(node);
        }else if(node->children[0]->type=="PRINT"){ //s-> PRINT  param_list  暂不进行错误检查
                // for(auto child:node->children)CHECK_POINT(child)
                vector<string> param_list;
                for(auto param:node->children[1]->children){//param_list:[e,e,e...]
                    analysis(param,-1); 
                    param_list.push_back(param->value);
                }
                string content=*(param_list.begin());
                auto p=param_list.begin()+1;

                int pindex=-2;
                for(int index=content.find("%");index!=string::npos;index=content.find("%")) 
                   { 
                        if(index-pindex>2){ 
                                //说明中间有字符
                            PUSH_CODE(global_code_num++,InterCode("ARG","_",content.substr(pindex+2,index-pindex-2),"STR"))
                            }    
                        //规定变量及常数控制符为%d,字符串为%s
                        if(content[index+1]=='d')PUSH_CODE(global_code_num++,InterCode("ARG","_",*p,"ID"))
                        else if(content[index+1]=='s')PUSH_CODE(global_code_num++,InterCode("ARG","_",*p,"STR"))
                        content.replace(index,1,"_");
                        p++;
                        pindex=index;
                   }
                if(pindex<content.size()-2)PUSH_CODE(global_code_num++,InterCode("ARG","_",content.substr(pindex+2),"STR"))
                PUSH_CODE(global_code_num++,InterCode("CALL","_","PRINT"))
        }else if(node->children.size()==1&&node->children[0]->type=="e")    //s->e;
            {   //for(auto child:node->children)CHECK_POINT(child)
                analysis(node->children[0],-1);
            }
        else if(node->children[0]->type=="RETURN"){
            if(node->children.size()==1)  //return;
                PUSH_CODE(global_code_num++,InterCode("RETURN","_"))
            else{   //return e
                analysis(node->children[1],-1);
                PUSH_CODE(global_code_num++,InterCode("RETURN","_",node->children[1]->value))
            }
        }else if(node->children[0]->type=="T"&&node->children[1]->type=="Aid_list"){ //数组定义  s->T Aid_list
            // for(auto child:node->children)CHECK_POINT(child)
            string arr_type=node->children[0]->value;
            for(auto aid:node->children[1]->children){//aidlist:[aid,aid,aid]
                string arr_name=aid->children[0]->name;//aid-> ID [NUM]  
                string arr_len=aid->children[1]->value;
               (*(table_manager.tables.end()-1))->add_arr(arr_name,arr_type,arr_len);
               (*(table_manager.tables.end()-1))->add_var(arr_name,arr_type+"*",arr_len);
            }
        }else DEFAULT_ACTION
    }else DEFAULT_ACTION
}   
//优化
void GrammarTree::optimize(){
   // 依次遍历每行中间代码
    for (int i = this->codes.size() - 2; i >= 0; i--) {
    // 删去未使用过的变量
    if (codes[i].first != -1 && codes[i].second.op=="=" && codes[i].second.result != "_"&&codes[i].second.op!="RETURN" 
        && codes[i].second.result[0] != 't') {   
      string name = this->codes[i].second.result;
      int sign = 0;
      for (int j = i + 1; j < this->codes.size()&&(sign==0);) 
      { 
            // 如果该中间代码已经被标记删去了则直接跳过
            if (this->codes[j].first == -1)
            {
                j++;
                continue;
            }
            // 下一句直接检查到赋值,中间序号中没有运算,说明重定义
            if (codes[j].second.op=="=" && codes[j].second.result== name)
                break;
            // 如果在后续的中间代码中使用到该变量则加入标记
            if (this->codes[j].second.arg1 == name ||
                this->codes[j].second.arg2 == name)
            {
                sign = 1;
                break;
            }
            // 跳转到下一跳中间代码
            if(this->codes[j].second.op == "GOTO" || this->codes[j].second.op[0] == 'J')
            {
                if (stoi(this->codes[j].second.result) < j) //若跳转到编号小的语句
                {
                    int target = stoi(this->codes[j].second.result);
                    for (int k = target;k < j;k++)  //从头检查
                    {
                        if (this->codes[k].first == -1) continue;
                        if (this->codes[k].second.arg1 == name ||this->codes[k].second.arg2 == name)
                            {
                                sign = 1;
                                break;
                            }
                    }
                    j++;
                }       
                else
                {
                    int target = stoi(this->codes[j].second.result);
                    for (int k = 0;k < this->codes.size();k++)
                    {
                        if (this->codes[k].first == target)
                        {
                            j = k;
                            break;
                        }
                    }
                }      
            }
            else
            j++;
      }
      // 如果该变量没有被使用过，那么直接将该变量的初始化语句删去
      if (sign == 0) 
      {
            this->codes[i].first = -1;
            // 调整该语句之后中间代码的序号
            for (int j = i+1;j < this->codes.size(); j++)
            {
                // 已经删除的中间代码直接跳过
                if(codes[j].first == -1)
                continue;

                this->codes[j].first =  this->codes[j].first-1;
            }
            // 调整跳转语句中的跳转序号
            for (int j = 0;j < this->codes.size(); j++)
            {
                // 已经删除的中间代码直接跳过
                if(codes[j].first == -1)
                continue;
                // 跳转语句
                if (this->codes[j].second.op == "GOTO" || this->codes[j].second.op == "J>" || this->codes[j].second.op == "J<" || this->codes[j].second.op == "JNE")
                {
                    if (stoi(this->codes[j].second.result) > i)
                    {
                        int temp = stoi(this->codes[j].second.result);
                        temp--;
                        this->codes[j].second.result = to_string(temp);
                    }
                }
            }
        }
    }
  }
  for (int i = this->codes.size() - 2; i >= 0; i--)
  {
      //（op,a,b,tmp）如果result在后面没使用，这段就要删掉（标号置为-1）
    if (codes[i].first != -1 && codes[i].second.op=="=" && codes[i].second.result != "_"&&codes[i].second.op!="RETURN" 
        && codes[i].second.result[0] == 't') {   
      string name = this->codes[i].second.result;
      int sign = 0;
      for (int j = i + 1; j < this->codes.size(); j++) {
          if(codes[j].first == -1)
            continue;
        if (codes[j].second.arg1 == name ||
            codes[j].second.arg2 == name) {
          sign = 1;
          break;
        }
      }
      if (sign == 0) {
            this->codes[i].first = -1;

            // 调整该语句之后中间代码的序号
            for (int j = i+1;j < this->codes.size(); j++)
            {
                // 已经删除的中间代码直接跳过
                if(codes[j].first == -1)
                continue;

                this->codes[j].first =  this->codes[j].first-1;
            }
            // 调整跳转语句中的跳转序号
            for (int j = 0;j < this->codes.size(); j++)
            {
                // 已经删除的中间代码直接跳过
                if(codes[j].first == -1)
                continue;
                // 跳转语句
                if (this->codes[j].second.op == "GOTO" || this->codes[j].second.op == "J>" || this->codes[j].second.op == "J<" || this->codes[j].second.op == "JNE")
                {
                    if (stoi(this->codes[j].second.result) > i)
                    {
                        int temp = stoi(this->codes[j].second.result);
                        temp--;
                        this->codes[j].second.result = to_string(temp);
                    }
                }
            }
      }
    }
  }
}
//整合中间代码
void GrammarTree::gen_intercode(Node *node) 
{
    if (node->code.size()) {
    for (int i = 0; i < node->code.size(); i++) {
      codes.push_back(node->code[i]);
    }
  }
  for (auto child : node->children) {
    gen_intercode(child);
  }
} 

bool cmp(const pair<int, InterCode> &p1, const pair<int, InterCode> &p2) {
  return p1.first < p2.first;
}
//中间代码打印到文件
void GrammarTree::print_code(){
 sort(codes.begin(), codes.end(), cmp);
    optimize();
  for (auto item : codes) {
    if (item.first != -1) {
      cout << item.first << ":\t" << item.second << endl;
    }
  }
} 