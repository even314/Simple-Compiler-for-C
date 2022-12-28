section .data
scale dw 10

section .bss
num resd 1
numbers resb 20
result resd 1
weight resd 1

section .text
	global _start
_start:



;指数运算
pow:     ;eax存计算结果(初始值为1)，ebx存底数，ecx存指数
    cmp ecx, 0
    jl label_pow1
    jg loop_pow
    ret
loop_pow:
    imul ebx
    loop loop_pow
    ret
label_pow1:
    mov eax, 0
    ret

;打印数字
PRINT:
   xor ecx, ecx      ;保存数字长度
   cmp eax, 0
   jge label_print_num
   not eax
   add eax, 01
   push eax
   mov [num], dword 45
   mov ecx, num      ;输出数字
   mov ebx, 1
   mov edx, 1
   mov eax, 4
   int 0x80
   pop eax
   xor ecx, ecx
label_print_num:
   xor edx, edx
   div word [scale]
   add edx, 48       ;edx中为余数
   push edx          ;从低到高转为字符并放入栈中
   inc ecx
   cmp eax, 00
   jne label_print_num
   mov esi, ecx   ;存放长度
   mov edx, 1     ;输出单个字符
label_print_num1:
   pop eax
   mov [num], eax
   mov ecx, num   ;输出数字
   mov ebx, 1
   mov eax, 4
   int 0x80
   dec esi
   cmp esi, 0
   jg label_print_num1
   ret


;输入数字，回车结束
INPUT:
   mov edx, 20
   mov ecx, numbers
   mov ebx, 0
   mov eax, 3
   int 80h
   xor esi, esi      ;存储输入数字位数
   xor edi, edi
   cmp [numbers], byte '-'
   jne input_label0
   mov edi, 01
   add ecx, 1
input_label0:
   xor eax, eax
   mov al, [ecx]
   cmp al, 0ah
   je input_label1
   sub eax, 48
   push eax
   inc esi
   add ecx, 1
   jmp input_label0
input_label1:
   mov [weight], dword 1
   mov [result], dword 0
input_label2:
   pop eax
   mul dword [weight]
   add [result], eax
   mov eax, [weight]
   mul dword [scale]
   mov [weight], eax
   dec esi
   cmp esi, 0
   jg input_label2
   cmp edi, 00
   je input_label3
   mov eax, [result]
   not eax
   add eax, 1
   mov [result], eax
input_label3:
   ret
    
