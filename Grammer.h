#pragma once
#include<string>

enum kinds { CONST, VAR, ARRAY, FUNC };
enum types { INT, CHAR, VOID };//1, 2, 0

typedef struct Entry {
	kinds kind;
	types type;
	int dimen;
	int offset;
	int dimenX = 0;
	int dimenY = 0;
	std::string value;
}Entry;

typedef struct Parameter {
	types type;
	std::string name;
}Parameter;

extern int preline;
extern std::map<std::string, std::map<std::string, Entry>> allLocalTable;
extern std::map<std::string, Entry> globalSymbolTable;
extern std::map<std::string, std::string> strTable;
extern std::map<std::string, std::vector<std::string>> globalArrayValue;

void grammer();
void noSymbolInteger(std::string* op = NULL);
void Integer(std::string* op = NULL);
types factor(std::string* op);
types item(std::string* op);
types expression(std::string* op);
void funcCallStatement();
void statement();
void statementlist();
void printTable();
void printLocal();
void printPara();
int string2int(std::string num);
void optimizeLocalTable();
