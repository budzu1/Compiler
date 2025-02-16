CXX = g++
CXXFLAGS = -std=c++17

EXEC = kompilator
BISON_FILE = parser.y
FLEX_FILE = lexer.l

BISON_OUT = parser.tab.cc
BISON_HDR = parser.tab.hh
FLEX_OUT = lex.yy.c

OBJS = main.o parser.tab.o lex.yy.o semantic_visitor.o codegen_visitor.o

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

parser.tab.o: $(BISON_OUT)
	$(CXX) $(CXXFLAGS) -c $(BISON_OUT) -o parser.tab.o

lex.yy.o: $(FLEX_OUT)
	$(CXX) $(CXXFLAGS) -c $(FLEX_OUT) -o lex.yy.o

$(BISON_OUT) $(BISON_HDR): $(BISON_FILE)
	bison -d -o $(BISON_OUT) $(BISON_FILE)

$(FLEX_OUT): $(FLEX_FILE)
	flex -o $(FLEX_OUT) $(FLEX_FILE)

main.o: main.cpp ast.hpp symtable.hpp semantic_visitor.hpp codegen_visitor.hpp
	$(CXX) $(CXXFLAGS) -c main.cpp -o main.o

semantic_visitor.o: semantic_visitor.cpp semantic_visitor.hpp symtable.hpp ast.hpp
	$(CXX) $(CXXFLAGS) -c semantic_visitor.cpp -o $@

codegen_visitor.o: codegen_visitor.cpp codegen_visitor.hpp ast.hpp symtable.hpp memory_manager.hpp
	$(CXX) $(CXXFLAGS) -c codegen_visitor.cpp -o $@

clean:
	rm -f $(EXEC) $(BISON_OUT) $(BISON_HDR) $(FLEX_OUT) *.o

.PHONY: all clean