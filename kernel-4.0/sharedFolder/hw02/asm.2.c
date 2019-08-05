#include <stdio.h>
int main(int argc, char** argv) {
    long int a=10;
    long int b=20;
    long int c=30;
    long int d=40;
    __asm__ volatile (  
    "mov $100, %%rax\n"   // rax = b
    "mov $200, %%rbx\n"
    "add %%rbx, %%rax\n"   // rax += a
    "mov %%rax, %0\n"   // b = rax
    : "=m" (b)  //output
    : "g" (a), "g" (d) //input
    : "rbx", "rax" //搞爛掉的暫存器
    );
    printf("a=%ld, b=%ld, c=%ld d=%ld\n", a, b, c, d);
}