#include<stdio.h>


int main(){
  int b[4];
  b[2] = 6;
  b[1] = 2;
  int c;
  c =  b[2];
  int d = b[1];
  c = c+d;
  printf("%d\n", c);
  return 0;
}
  