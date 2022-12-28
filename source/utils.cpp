#include"../include/utils.h"

void print_red1(string s){
    cout<<"\033[31m"<<s<<"\033[0m"<<endl;
}

void print_red2(string s){
    cerr<<"\033[31m"<<s<<"\033[0m"<<endl;
}

void print_yellow1(string s){
    cout<<"\033[33m"<<s<<"\033[0m"<<endl;
}

void print_yellow2(string s){
    cerr<<"\033[33m"<<s<<"\033[0m"<<endl;
}
