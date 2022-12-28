section .data
scale dw 10
msg45 db '',0xa
len45 equ $ - msg45

section .bss
num resd 1
numbers resb 20
result resd 1
weight resd 1
temphh1 resd 1
a resd 1
temphh2 resd 1
temphh4 resd 1
temphh3 resd 1
sum resd 1
i resd 1
temphh5 resd 1
temphh6 resd 1
j resd 1
temphh7 resd 1
temphh8 resd 1
temphh9 resd 1
temphh10 resd 1
temphh11 resd 1
temphh12 resd 1
temphh13 resd 1
temphh14 resd 1
temphh15 resd 1
temphh16 resd 1
temphh17 resd 1
temphh19 resd 1
temphh21 resd 1

section .text
	global _start
_start:
	mov eax, 10
	mov [temphh1], eax
	mov [a], eax
	mov eax, 1
	mov [temphh2], eax
	mov eax, 0
	mov [temphh4], eax
	sub eax, [temphh2]
	mov [temphh3], eax
	mov [sum], eax
	mov eax, [temphh4]
	mov [i], eax
label8:
	mov eax, 2
	mov [temphh5], eax
	imul dword [a]
	mov [temphh6], eax
	mov ebx, [i]
	cmp ebx, eax
	jl label12
	jmp label44
label12:
	mov eax, [temphh4]
	mov [j], eax
label13:
	mov eax, [j]
	mov ebx, [a]
	cmp eax, ebx
	jl label15
	jmp label41
label15:
	mov eax, [a]
	xor edx, edx
	cmp eax, 00
	jge div_label0
	not edx
div_label0:
	mov ebx, [temphh5]
	idiv ebx
	mov [temphh7], eax
	mov ebx, [j]
	cmp ebx, eax
	jl label18
	jmp label28
label18:
	mov eax, [j]
	xor edx, edx
	cmp eax, 00
	jge div_label1
	not edx
div_label1:
	mov ebx, [temphh5]
	idiv ebx
	mov [temphh8], edx
	mov eax, [temphh8]
	mov ebx, [temphh4]
	cmp eax, ebx
	jne label21
	jmp label28
label21:
	mov eax, [i]
	imul dword [j]
	mov [temphh9], eax
	xor edx, edx
	cmp eax, 00
	jge div_label2
	not edx
div_label2:
	mov ebx, [temphh5]
	idiv ebx
	mov [temphh10], eax
	add eax, [sum]
	mov [temphh11], eax
	mov eax, [i]
	imul dword [j]
	mov [temphh12], eax
	add eax, [temphh11]
	mov [temphh13], eax
	mov [sum], eax
	jmp label38
label28:
	mov eax, [sum]
	mov ebx, [j]
	cmp eax, ebx
	jl label33
	jmp label30
label30:
	mov eax, [sum]
	xor edx, edx
	cmp eax, 00
	jge div_label3
	not edx
div_label3:
	mov ebx, [temphh5]
	idiv ebx
	mov [temphh14], edx
	mov eax, [temphh14]
	mov ebx, [temphh2]
	cmp eax, ebx
	je label38
	jmp label33
label33:
	mov eax, 3
	mov [temphh15], eax
	xor edx, edx
	cmp eax, 00
	jge div_label4
	not edx
div_label4:
	mov ebx, [temphh5]
	idiv ebx
	mov [temphh16], eax
	add eax, [sum]
	mov [temphh17], eax
	mov [sum], eax
	jmp label28
label38:
	mov eax, [j]
	add eax, [temphh2]
	mov [temphh19], eax
	mov [j], eax
	jmp label13
label41:
	mov eax, [i]
	add eax, [temphh2]
	mov [temphh21], eax
	mov [i], eax
	jmp label8
label44:
	mov eax, [sum]
	call PRINT
	mov eax, 4
	mov ebx, 1
	mov ecx, msg45
	mov edx, len45
	int 80h
	mov eax, 1
	int 0x80



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
    
