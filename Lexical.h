#pragma once

#include<iostream>
#include<fstream>
#include<unordered_map>
#include<algorithm>
#include<vector>
#include<map>

enum Type_set {
	//标识符  整形常量 字符常量  字符串   const    int    char    void    main    if
	IDENFR, INTCON, CHARCON, STRCON, CONSTTK, INTTK, CHARTK, VOIDTK, MAINTK, IFTK,
	//else    switch    case    default    while    for    scanf    printf    return
	ELSETK, SWITCHTK, CASETK, DEFAULTTK, WHILETK, FORTK, SCANFTK, PRINTFTK, RETURNTK,
	//+	   -     *     /
	PLUS, MINU, MULT, DIV,
	//<   <=    >   >=   ==   !=     =	
	LSS, LEQ, GRE, GEQ, EQL, NEQ, ASSIGN,
	//:      ;       ,       (        )        [       ]       {       }
	COLON, SEMICN, COMMA, LPARENT, RPARENT, LBRACK, RBRACK, LBRACE, RBRACE
};

extern std::ifstream filein;
extern std::ofstream fileout;
extern std::ofstream mips;
extern std::string buf;
extern Type_set symbol;
extern std::vector<std::string> record;

void getSym();
void printSym();
void open();
void close();
void error(std::string reason);
void errorHandle(char type);
void printRecord();
void errorHandleInLexical(char type);
std::string enum2str(Type_set type);
