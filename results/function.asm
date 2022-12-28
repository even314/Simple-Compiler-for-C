section .data
scale dw 10
msg26 db '',0xa
len26 equ $ - msg26

section .bss
num resd 1
numbers resb 20
result resd 1
weight resd 1
a resd 1
b resd 1
temphh1 resd 1
temphh2 resd 1
temphh3 resd 1
temphh4 resd 1
temphh5 resd 1
temphh7 resd 1
temphh6 resd 1
temphh8 resd 1
temphh9 resd 1
temphh10 resd 1
temphh11 resd 1
temphh12 resd 1
temphh13 resd 1
temphh14 resd 1

section .text
	global _start
_start:
	mov eax, 1
	mov [temphh12], eax
	mov eax, [temphh12]
	mov ebx, [temphh12]
	call fib
	mov eax, [result]
	mov [temphh13], eax
	call PRINT
	mov eax, 4
	mov ebx, 1
	mov ecx, msg26
	mov edx, len26
	int 80h
	mov eax, 0
	mov [temphh14], eax
	mov eax, 1
	int 0x80


fib:
	mov [a], eax
	mov [b], ebx
	mov eax, 1000
	mov [temphh1], eax
	mov ebx, [a]
	cmp ebx, eax
	jg label6
	jmp label14
label6:
	mov eax, [a]
	imul dword [b]
	mov [temphh2], eax
	mov eax, 5
	mov [temphh3], eax
	mov eax, [temphh2]
	xor edx, edx
	cmp eax, 00
	jge div_label0
	not edx
div_label0:
	mov ebx, [temphh3]
	idiv ebx
	mov [temphh4], edx
	mov eax, 1
	mov [temphh5], eax
	mov eax, 0
	mov [temphh7], eax
	sub eax, [temphh5]
	mov [temphh6], eax
	mov eax, [temphh4]
	sub eax, [temphh6]
	mov [temphh8], eax
	mov [result], eax
	ret
label14:
	mov eax, [a]
	add eax, [b]
	mov [temphh9], eax
	mov eax, [temphh9]
	mov ebx, [a]
	call fib
	mov eax, [result]
	mov [temphh10], eax
	add eax, [temphh5]
	mov [temphh11], eax
	mov [result], eax
	ret

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
    
