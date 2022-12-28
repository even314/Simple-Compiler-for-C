#include<stdio.h>

int main() {
    int a = 1, b = 1;
    int c;

    // Fibonacci
    for (int i = 2; i < 20; i++) {
        c = a;
        a = a + b;
        b = c;
    }

    printf("%d %d\n", a, b);
    return 0;
}