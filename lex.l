%option noyywrap yylineno

%{
#include<iostream>
#include<cstring>
#include "parse.tab.h"

using namespace std;
int colno = 0;

extern "C"{
    extern int yylex();
    extern int yyerror(const char* str);
}

%}

%s COMMENT

%%

"#include".*        { /*忽略头文件*/ }
"//"[^\n]*          { /*这一部分匹配单行及多行注释*/ }
"/*"                { 
                        colno += yyleng;
                        BEGIN COMMENT;
                    }
<COMMENT>"*/"       {
                        colno += yyleng;
                        BEGIN INITIAL;
                    }
<COMMENT>.+/"*/"    {   colno += yyleng;  }
<COMMENT>\n         {
                        colno = 1;
                    }


[\r\t\v\f ]         {   colno++;   }
\n                  {   colno = 1; }


"int"   |
"void"              {
                        yylval.type = (char*)malloc(yyleng + 1);
                        strcpy(yylval.type, yytext);
                        colno += yyleng;
                        return T;
                    }


"if"                {
                        colno += 2;
                        return IF;
                    }
"else"              {
                        colno += 4;
                        return ELSE;
                    }
"while"             {
                        colno += 5; 
                        return WHILE;
                    }
"for"               {
                        colno += 3;
                        return FOR;
                    }
"return"            {
                        colno += 6;
                        return RETURN;
                    }


"++"                {
                        colno += 2;
                        return INC;
                    }
"--"                {
                        colno += 2;
                        return DEC;
                    }
"<="                {
                        colno += 2;
                        return LE;
                    }
">="                {
                        colno += 2;
                        return GE;
                    }
"=="                {
                        colno += 2;
                        return EQ;
                    }
"!="                {
                        colno += 2;
                        return NE;
                    }

"&&"                {
                        colno += 2;
                        return AND;
                    }
"||"                {
                        colno += 2;
                        return OR;
                    }


"!" |
"-" |
"+" |
"*" |
"/" |
"%" |
"<" |
">" |
"^" |
"=" |
"[" |
"]" |
"," |
";" |
"{" |
"}" |
"(" |
")"                 {   //运算符和括号
                        colno++;
                        return yytext[0];
                    }


"printf"            {
                        colno += 6;
                        return PRINT;
                    }
"scanf"             {
                        colno += 5;
                        return INPUT;
                    }
"bool" |
"true" |
"false"             {
                        cout << "行: " << yylineno << ", 列: " << colno << ", 不支持布尔类型" << endl;
                        exit(-1);
                    }
"float" |
[0-9]*\.[0-9]+      {
                        cout << "行: " << yylineno << ", 列: " << colno << ", 不支持浮点数类型" << endl;
                        exit(-1);
                    }
"char" |
'.'                 {
                        cout << "行: " << yylineno << ", 列: " << colno << ", 不支持字符类型" << endl;
                        exit(-1);
                    }


[_a-zA-Z][_0-9a-zA-Z]*  {   //标识符，将名字(id.name)返回给语法分析器
                            yylval.id.name = (char*)malloc(yyleng + 1);
                            strcpy(yylval.id.name, yytext);
                            colno += yyleng;
                            return ID;
                        }
[0-9]+                  {   //整数常量，将值(value)以字符串的形式返回给语法分析器
                            yylval.value = (char*)malloc(yyleng + 1);
                            strcpy(yylval.value, yytext);
                            colno += yyleng;
                            return NUM;
                        }
\"[^"]+?\"              {
                            //字符串(str)
                            yylval.str = (char*)malloc(yyleng - 1);
                            strncpy(yylval.str, yytext + 1, yyleng - 2);
                            colno += yyleng;
                            return STR;
                        }


[0-9][a-zA-Z0-9_]+      {
                            cout << "行: " << yylineno << ", 列: " << colno << ", 非法标识符" << endl;
                            exit(-1);
                        }
.                       {
                            cout << "行: " << yylineno << ", 列: " << colno << ", 不支持的符号" << endl;
                            exit(-1);
                        }

%%