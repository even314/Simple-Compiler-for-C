run:
	flex ../lex.l
	bison -dv ../parse.y
	g++ -o run -g lex.yy.c parse.tab.c ../source/SymbolTable.cpp ../source/GrammarTree.cpp ../source/utils.cpp -std=c++11
	
	cd ..
	
	./run ../program/wrong.c
