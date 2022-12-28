%{
#include<iostream>
#include<cstdio>
#include<string>
#include<cstring>
#include<stack>
#include "../include/SymbolTable.h"
#include "../include/GrammarTree.h"

using namespace std;

extern FILE* yyin;
extern int yylineno;

extern "C"{
    extern int yyerror(const char*);
    extern int yylex();
    extern int yyparse();
}

Node* child=NULL;
stack<Node*> statements;
#define ADD_NODE child=statements.top();statements.pop();node->add_child(child);
#define ADD_NEW_NODE(type, any) child=new Node(type, any);node->add_child(child);
#define ADD_TO_STACK statements.push(node);
%}

%union{
    char* type;
    char* value;
    char* str;
    struct{
        char *name;
        char* type;
        char *value;
        int address;
    } id;
}

%token <id> ID
%token <value> NUM
%token <str> STR
%token <type> T

%token INC DEC LE GE EQ NE AND OR
%token IF ELSE WHILE FOR RETURN PRINT INPUT

%nonassoc nif
%nonassoc ELSE

%right '='
%left OR
%left AND
%left EQ NE
%left '>' GE '<' LE
%left '+' '-'
%left '*' '/' '%'
%right '!' INC DEC
%left '^'
%nonassoc UMINUS


%%
all
    : p_list      {
            Node* node = new Node("all");
            while(statements.size()){
                node->add_child(statements.top());
                statements.pop();
            }
            grammar_tree.root = node;
            grammar_tree.trim(node);
            grammar_tree.analysis(node, -1);
            grammar_tree.gen_intercode(node);
            grammar_tree.print_code();
    }
    ;

p_list
    : p p_list 
    | s p_list 
    | s  
    | p     
    ;

p
    :T ID '(' dec_param_list ')' '{' s_list '}' {
                                                    Node* node = new Node("p");
                                                    ADD_NODE
                                                    ADD_NODE
                                                    ADD_NEW_NODE("ID", $2.name)
                                                    ADD_NEW_NODE("T", $1)
                                                    ADD_TO_STACK
                                                }
    | T ID  '(' ')' '{' s_list  '}'             {
                                                    Node* node = new Node("p");
                                                    ADD_NODE
                                                    ADD_NEW_NODE("ID", $2.name)
                                                    ADD_NEW_NODE("T", $1)
                                                    ADD_TO_STACK
                                                }
    ;

s_list
    : s s_list                                  {
                                                    Node* node = new Node("s_list");
                                                    ADD_NODE
                                                    ADD_NODE
                                                    ADD_TO_STACK
                                                }
    | s                                         {
                                                    Node* node = new Node("s_list");
                                                    ADD_NODE
                                                    ADD_TO_STACK
                                                }

s
    : IF '(' e ')' '{' s_list '}'  %prec nif             {
                                                            Node* node = new Node("s");
                                                            ADD_NODE
                                                            ADD_NODE
                                                            ADD_NEW_NODE("IF", "")
                                                            ADD_TO_STACK
                                                        }
    | IF '(' e ')' '{' s_list '}' ELSE '{' s_list '}'   {
                                                            Node* node = new Node("s");
                                                            ADD_NODE
                                                            ADD_NEW_NODE("ELSE", "")
                                                            ADD_NODE
                                                            ADD_NODE
                                                            ADD_NEW_NODE("IF", "")
                                                            ADD_TO_STACK
                                                        }
    | WHILE '(' e ')' '{' s_list '}'                    {
                                                            Node* node = new Node("s");
                                                            ADD_NODE
                                                            ADD_NODE
                                                            ADD_NEW_NODE("WHILE","")
                                                            ADD_TO_STACK
                                                        }
    | FOR '(' e ';' e ';' e ')' '{' s_list '}'          {
                                                            Node* node = new Node("s");
                                                            ADD_NODE
                                                            ADD_NODE
                                                            ADD_NODE
                                                            ADD_NODE
                                                            ADD_NEW_NODE("FOR", "")
                                                            ADD_TO_STACK
                                                        }
    | T Aid_list ';'                                    {
                                                            Node* node = new Node("s");
                                                            ADD_NODE
                                                            ADD_NEW_NODE("T", $1)
                                                            ADD_TO_STACK
                                                        }
    | PRINT '(' param_list ')' ';'                      {
                                                            Node* node = new Node("s");
                                                            ADD_NODE
                                                            ADD_NEW_NODE("PRINT", "")
                                                            ADD_TO_STACK
                                                        }
    | e ';'                                             {
                                                            Node* node = new Node("s");
                                                            ADD_NODE
                                                            ADD_TO_STACK
                                                        }
    | RETURN e ';'                                      {
                                                            Node* node = new Node("s");
                                                            ADD_NODE
                                                            ADD_NEW_NODE("RETURN","")
                                                            ADD_TO_STACK
                                                        }
    ;

dec_param_list
    : dec_param ',' dec_param_list      {
                                            Node* node = new Node("dec_param_list");
                                            ADD_NODE
                                            ADD_NODE
                                            ADD_TO_STACK
                                        }
    | dec_param                         {
                                            Node* node = new Node("dec_param_list");
                                            ADD_NODE
                                            ADD_TO_STACK
                                        }
    ;

dec_param
    : T ID                              {
                                            Node* node= new Node("dec_param");
                                            ADD_NEW_NODE("ID", $2.name);
                                            ADD_NEW_NODE("T", $1);
                                            ADD_TO_STACK
                                        }
    ;

param_list
    : e ',' param_list                  {
                                            Node* node = new Node("param_list");
                                            ADD_NODE
                                            ADD_NODE
                                            ADD_TO_STACK
                                        }
    | e                                 {
                                            Node* node = new Node("param_list");
                                            ADD_NODE
                                            ADD_TO_STACK
                                        }
    ;

Aid_list
    : Aid ',' Aid_list                  {
                                            Node* node = new Node("Aid_list");
                                            ADD_NODE
                                            ADD_NODE
                                            ADD_TO_STACK
                                        }
    | Aid                               {
                                            Node* node = new Node("Aid_list");
                                            ADD_NODE
                                            ADD_TO_STACK
                                        }
    ;

Aid
    : ID '[' NUM ']'                    {
                                            Node* node = new Node("Aid");
                                            ADD_NEW_NODE("NUM", $3)
                                            ADD_NEW_NODE("ID", $1.name)
                                            ADD_TO_STACK
                                        }
    ;

Did_list
    : Did ',' Did_list                  {
                                            Node* node = new Node("Did_list");
                                            ADD_NODE
                                            ADD_NODE
                                            ADD_TO_STACK
                                        }
    | Did                               {
                                            Node* node = new Node("Did_list");
                                            ADD_NODE
                                            ADD_TO_STACK
                                        }
    ;

Did
    :ID                                 {
                                            Node* node = new Node("Did");
                                            ADD_NEW_NODE("ID", $1.name);
                                            ADD_TO_STACK
                                        }
    | ID '=' e                          {
                                            Node* node = new Node("Did");
                                            ADD_NODE
                                            ADD_NEW_NODE("=", "")
                                            ADD_NEW_NODE("ID", $1.name)
                                            ADD_TO_STACK
                                        }
    ;

e
    : INPUT '(' ')'         {
                                Node* node = new Node("e");
                                ADD_NEW_NODE(")", "")
                                ADD_NEW_NODE("(", "")
                                ADD_NEW_NODE("INPUT", "")
                                ADD_TO_STACK
                            }
    | T Did_list            {
                                Node* node = new Node("e");
                                ADD_NODE
                                ADD_NEW_NODE("T", $1)
                                ADD_TO_STACK
                            }
    | e OR e                {
                                Node* node = new Node("e");
                                ADD_NODE
                                ADD_NEW_NODE("OR", "")
                                ADD_NODE
                                ADD_TO_STACK
                            }

    | e AND e               {
                                Node* node = new Node("e");
                                ADD_NODE
                                ADD_NEW_NODE("AND", "")
                                ADD_NODE
                                ADD_TO_STACK
                            }
    | '!' e                 {
                                Node* node = new Node("e");
                                ADD_NODE
                                ADD_NEW_NODE("!", "")
                                ADD_TO_STACK
                            }
    | ID '(' param_list ')' {
                                Node* node = new Node("e");
                                ADD_NEW_NODE(")", "")
                                ADD_NODE
                                ADD_NEW_NODE("(", "")
                                ADD_NEW_NODE("ID", $1.name)
                                ADD_TO_STACK
                                                   
                            }
    | ID '('  ')'           {
                                Node* node = new Node("e");
                                ADD_NEW_NODE(")", "")
                                ADD_NEW_NODE("(", "")
                                ADD_NEW_NODE("ID", $1.name)
                                ADD_TO_STACK
                            }       
    | '(' e ')'             {
                                Node* node = new Node("e");
                                ADD_NEW_NODE(")", "")
                                ADD_NODE
                                ADD_NEW_NODE("(", "")
                                ADD_TO_STACK
                            }
    | e '<' e               {
                                Node* node = new Node("e");
                                ADD_NODE
                                ADD_NEW_NODE("<", "")
                                ADD_NODE
                                ADD_TO_STACK
                            }
    | e '>' e               {
                                Node* node = new Node("e");
                                ADD_NODE
                                ADD_NEW_NODE(">", "")
                                ADD_NODE
                                ADD_TO_STACK
                            }
    | e LE e                {
                                Node* node = new Node("e");
                                ADD_NODE
                                ADD_NEW_NODE("LE", "")
                                ADD_NODE
                                ADD_TO_STACK
                            }
    | e GE e                {
                                Node* node = new Node("e");
                                ADD_NODE
                                ADD_NEW_NODE("GE", "")
                                ADD_NODE
                                ADD_TO_STACK
                            }
    | e EQ e                {
                                Node* node = new Node("e");
                                ADD_NODE
                                ADD_NEW_NODE("EQ", "")
                                ADD_NODE
                                ADD_TO_STACK
                            }
    | e NE e                {
                                Node* node = new Node("e");
                                ADD_NODE
                                ADD_NEW_NODE("NE", "")
                                ADD_NODE
                                ADD_TO_STACK
                            }
    | e '+' e               {
                                Node* node = new Node("e");
                                ADD_NODE
                                ADD_NEW_NODE("+", "")
                                ADD_NODE
                                ADD_TO_STACK
                            }
    | e '-' e               {
                                Node* node = new Node("e");
                                ADD_NODE
                                ADD_NEW_NODE("-", "")
                                ADD_NODE
                                ADD_TO_STACK
                            }
    | e '*' e               {
                                Node* node = new Node("e");
                                ADD_NODE
                                ADD_NEW_NODE("*", "")
                                ADD_NODE
                                ADD_TO_STACK
                            }
    | e '/' e               {
                                Node* node = new Node("e");
                                ADD_NODE
                                ADD_NEW_NODE("/", "")
                                ADD_NODE
                                ADD_TO_STACK
                            }
    | e '%' e               {
                                Node* node = new Node("e");
                                ADD_NODE
                                ADD_NEW_NODE("%", "")
                                ADD_NODE
                                ADD_TO_STACK
                            }
    | e '^' e               {
                                Node* node = new Node("e");
                                ADD_NODE
                                ADD_NEW_NODE("^", "")
                                ADD_NODE
                                ADD_TO_STACK
                            }
    | '-' e  %prec UMINUS   {
                                Node* node = new Node("e");
                                ADD_NODE
                                ADD_NEW_NODE("-", "")
                                ADD_TO_STACK
                            }
    | e '=' e               {
                                Node* node = new Node("e");
                                ADD_NODE
                                ADD_NEW_NODE("=", "")
                                ADD_NODE
                                ADD_TO_STACK
                            }
    | NUM                   {
                                Node* node = new Node("e");
                                ADD_NEW_NODE("NUM",  $1)
                                ADD_TO_STACK
                            }
    | STR                   {
                                Node* node = new Node("e");
                                ADD_NEW_NODE("STR", $1)
                                ADD_TO_STACK
                            }
    | ID                    {
                                Node* node = new Node("e");
                                ADD_NEW_NODE("ID", $1.name)
                                ADD_TO_STACK
                            }
    | e INC                 {
                                Node* node = new Node("e");
                                ADD_NEW_NODE("INC","")
                                ADD_NODE
                                ADD_TO_STACK
                            }
    | e DEC                 {
                                Node* node = new Node("e");
                                ADD_NEW_NODE("DEC","")
                                ADD_NODE
                                ADD_TO_STACK
                            }
    | INC e                 {
                                Node* node = new Node("e");
                                ADD_NODE
                                ADD_NEW_NODE("INC","")
                                ADD_TO_STACK
                            }
    | DEC e                 {
                                Node* node = new Node("e");
                                ADD_NODE
                                ADD_NEW_NODE("DEC","")
                                ADD_TO_STACK
                            }
    | ID '[' e ']'          {
                                Node* node = new Node("e");
                                ADD_NEW_NODE("]","")
                                ADD_NODE
                                ADD_NEW_NODE("[", "")
                                ADD_NEW_NODE("ID", $1.name)
                                ADD_TO_STACK
                            }
    ;

%%

int main(int argc, char **argv){
    FILE* fp = fopen(argv[1], "r");
    yyin = fp;
    yyparse();
    fclose(fp);
    return 0;
}

int yyerror(const char * str){
    cout << "Error: " << str << endl;
    return 0;
}
