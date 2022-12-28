import sys

eax = []        # eax 寄存器保存临时数据

labels = {}     # 需要添加的标签集合

variables = ['section .bss', 'num resd 1', 'numbers resb 20', 'result resd 1', 'weight resd 1']        # 变量列表

constants = ['section .data\n', 'scale dw 10\n']    # 常量列表

main_codes = ['section .text\n\tglobal _start\n_start:\n']   # 代码区

func_codes = []

registers = ['eax', 'ebx', 'ecx', 'edx', 'esi', 'edi']

skip = { 
    'J<' : 'jl',
    'JNE' : 'jne',
    'JEQ' : 'je',
    'J>' : 'jg'
}

div_label = 0   # 除法需要跳转
line = 0


def assign(arg1, arg2, result, codes):
    if arg1 in eax:
        eax.append(result)
    else:
        eax.clear()
        eax.append(result)
        if '#' in arg1:
            codes.append(f'\tmov eax, {arg1[1:]}\n')
        else:
            eax.append(arg1)
            codes.append(f'\tmov eax, [{arg1}]\n')
    codes.append(f'\tmov [{result}], eax\n')


def add(arg1, arg2, result, codes):
    if arg2 in eax:
        codes.append(f'\tadd eax, [{arg1}]\n')
    else:
        if arg1 not in eax:
            codes.append(f'\tmov eax, [{arg1}]\n')
        codes.append(f'\tadd eax, [{arg2}]\n')
    eax.clear()
    eax.append(result)
    codes.append(f'\tmov [{result}], eax\n')


def sub(arg1, arg2, result, codes):
    if arg1 not in eax:
        codes.append(f'\tmov eax, [{arg1}]\n')
    codes.append(f'\tsub eax, [{arg2}]\n')
    eax.clear()
    eax.append(result)
    codes.append(f'\tmov [{result}], eax\n')


def mul(arg1, arg2, result, codes):
    if arg2 in eax:
        codes.append(f'\timul dword [{arg1}]\n')
    else:
        if arg1 not in eax:
            codes.append(f'\tmov eax, [{arg1}]\n')
        codes.append(f'\timul dword [{arg2}]\n')
    eax.clear()
    eax.append(result)
    codes.append(f'\tmov [{result}], eax\n')


def div(arg1, arg2, result, codes):
    global div_label
    if arg1 not in eax:
        codes.append(f'\tmov eax, [{arg1}]\n')
    codes.append(f'\txor edx, edx\n\tcmp eax, 00\n\tjge div_label{div_label}\n\tnot edx\ndiv_label{div_label}:\n\tmov ebx, [{arg2}]\n\tidiv ebx\n\tmov [{result}], eax\n')
    eax.clear()
    eax.append(result)
    div_label += 1


def mod(arg1, arg2, result, codes):
    global div_label
    if arg1 not in eax:
        codes.append(f'\tmov eax, [{arg1}]\n')
    codes.append(f'\txor edx, edx\n\tcmp eax, 00\n\tjge div_label{div_label}\n\tnot edx\ndiv_label{div_label}:\n\tmov ebx, [{arg2}]\n\tidiv ebx\n\tmov [{result}], edx\n')
    eax.clear()
    div_label += 1


def pow(arg1, arg2, result, codes):
    eax.clear()
    codes.append(f'\tmov eax, 1\n\tmov ebx, [{arg1}]\n\tmov ecx, [{arg2}]\n\tcall pow\n\tmov [{result}], eax\n')
    eax.append(result)


def inc(arg1, arg2, result, codes):
    codes.append(f'\tinc dword [{result}]\n')


def dec(arg1, arg2, result, codes):
    codes.append(f'\tdec dword [{result}]\n')


def cmp_jump(arg1, arg2, result, way, codes):
    if arg2 in eax:
        codes.append(f'\tmov ebx, [{arg1}]\n\tcmp ebx, eax\n\t{way} label{result}\n')
    else:
        if arg1 not in eax:
            codes.append(f'\tmov eax, [{arg1}]\n')
        codes.append(f'\tmov ebx, [{arg2}]\n\tcmp eax, ebx\n\t{way} label{result}\n')


operation = {
    '+' : add,
    '-' : sub,
    '*' : mul,
    '/' : div,
    '%' : mod,
    '^' : pow,
    '=' : assign,
    'INC' : inc,
    'DEC' : dec
}

def main(inter_codes):
    global line
    arguments = 0
    while line < len(inter_codes):
        location, operator, arg1, arg2, result = inter_codes[line]
        if operator == 'RETURN':
            line += 1
            break
        if location in labels:
            eax.clear()
            main_codes.append(f'label{location}:\n')
        if operator in operation:
            if operator in ['=', 'INC', 'DEC'] and f'{result} resd 1' not in variables:
                 variables.append(f'{result} resd 1')
            if operator not in ['=', 'INC', 'DEC']:
                for i in [arg1, arg2, result]:
                    if f'{i} resd 1' not in variables:
                        variables.append(f'{i} resd 1')
            operation[operator](arg1, arg2, result, main_codes)
        if operator in skip:
            cmp_jump(arg1, arg2, result, skip[operator], main_codes)
            eax.clear()
        if operator == 'GOTO':
            eax.clear()
            main_codes.append(f'\tjmp label{result}\n')
        if operator == 'ARG':
            if arg2 == 'STR':
                eax.clear()
                if arg1.endswith('\\n'):
                    constants.append(f'msg{location} db \'{arg1[:-2]}\',0xa\nlen{location} equ $ - msg{location}\n')
                else:
                    constants.append(f'msg{location} db \'{arg1}\'\nlen{location} equ $ - msg{location}\n')
                main_codes.append(f'\tmov eax, 4\n\tmov ebx, 1\n\tmov ecx, msg{location}\n\tmov edx, len{location}\n\tint 80h\n')
            elif arg2 == 'ID':
                if arg1 not in eax:
                    main_codes.append(f'\tmov eax, [{arg1}]\n')
                main_codes.append('\tcall PRINT\n')
                eax.clear()
            else:
                main_codes.append(f'\tmov {registers[arguments]}, [{arg1}]\n')
                arguments += 1
        if operator == 'CALL' and arg1 != 'PRINT':
            eax.clear()
            arguments = 0
            main_codes.append(f'\tcall {arg1}\n')
            if result != '_':
                if f'{result} resd 1' not in variables:
                    variables.append(f'{result} resd 1')
                main_codes.append(f'\tmov eax, [result]\n\tmov [{result}], eax\n')
                eax.append(result)
        line += 1
    main_codes.append(f'\tmov eax, 1\n\tint 0x80\n')


def define(inter_codes):
    global line
    name = inter_codes[line][4]
    func_codes.append(f'{name}:\n')
    vars = []           # 在函数中出现的变量
    line += 1
    parameters = 0
    arguments = 0
    while line < len(inter_codes):   
        location, operator, arg1, arg2, result = inter_codes[line]
        if operator == 'FUNCTION':
            return
        if operator == 'param':
            if f'{result} resd 1' not in variables:
                variables.append(f'{result} resd 1')
            vars.append(result)
            func_codes.append(f'\tmov [{result}], {registers[parameters]}\n')
            parameters += 1
        if location in labels:
            eax.clear()
            func_codes.append(f'label{location}:\n')
        if operator in operation:
            t = f'{result} resd 1'
            if t not in variables:
                variables.append(t)
            if result not in vars:
                vars.append(result)
            operation[operator](arg1, arg2, result, func_codes)
        if operator in skip:
            cmp_jump(arg1, arg2, result, skip[operator], func_codes)
            eax.clear()
        if operator == 'GOTO':
            eax.clear()
            func_codes.append(f'\tjmp label{result}\n')
        if operator == 'ARG':
            if arg2 == 'STR':
                eax.clear()
                constants.append(f'msg{location} db \'{arg1}\'\nlen{location} equ $ - msg{location}\n')
                main_codes.append(f'\tmov eax, 4\n\tmov ebx, 1\n\tmov ecx, msg{location}\n\tmov edx, len{location}\n\tint 80h\n')
            elif arg2 == 'ID':
                if arg1 not in eax:
                    main_codes.append(f'\tmov eax, [{arg1}]\n')
                main_codes.append('\tcall PRINT\n')
                eax.clear()
            else:
                func_codes.append(f'\tmov {registers[arguments]}, [{arg1}]\n')
                arguments += 1
        if operator == 'CALL' and arg1 != 'PRINT':
            arguments = 0
            # if arg1 == name:        # 递归需要把变量压入栈中
            #     for i in vars:
            #         func_codes.append(f'\tpush dword [{i}]\n')
            #     vars.reverse()
            func_codes.append(f'\tcall {arg1}\n')
            eax.clear()
            # if arg1 == name:
            #     for i in vars:
            #         func_codes.append(f'\tpop dword [{i}]\n')
            if result != '_':
                if f'{result} resd 1' not in variables:
                    variables.append(f'{result} resd 1')
                func_codes.append(f'\tmov eax, [result]\n\tmov [{result}], eax\n')
                
                eax.append(result)
        if operator == 'RETURN':
            if arg1 not in eax:
                func_codes.append(f'\tmov eax, [{arg1}]\n')
            func_codes.append(f'\tmov [result], eax\n\tret\n')
            eax.clear()
        line += 1
            

if __name__ == '__main__':
    path = sys.argv[1]
    with open(path, 'r', encoding='utf-8') as f:
        inter_codes = f.read().split('\n')[:-1]
        for i in range(len(inter_codes)):
            inter_codes[i] = inter_codes[i].split('\t')
            inter_codes[i][0] = inter_codes[i][0][:-1]
        labels = { i[4] for i in inter_codes if i[1] in ['J<', 'JNE', 'GOTO', 'J>', 'JEQ'] }
        while line < len(inter_codes):
            if inter_codes[line][4] == 'main':
                line += 1
                main(inter_codes)
            else:
                define(inter_codes)
            
        with open('./assemble/functions.asm', 'r', encoding='utf-8') as func:
            print(''.join(constants) + '\n' + '\n'.join(variables) + '\n\n' + ''.join(main_codes) + '\n\n' + ''.join(func_codes) + '\n' + func.read())
