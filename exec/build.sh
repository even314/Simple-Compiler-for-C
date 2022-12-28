file="case1_1"
flex ../lex.l
bison -dv ../parse.y
g++ -o run -g lex.yy.c parse.tab.c ../source/GrammarTree.cpp ../source/SymbolTable.cpp ../source/utils.cpp -std=c++11

cd ..

./exec/run ./program/wrong/$file.c > ./results/$file.txt

python3 ./assemble/transfer.py ./results/$file.txt > ./results/$file.asm
nasm -f elf ./results/$file.asm
ld -m elf_i386 -s -o ./results/$file ./results/$file.o
rm ./results/$file.o

./results/$file
