#include "Lexical.h"
#include "Grammer.h"
#include "Mips.h"
#include "Intercode.h"

#include<iostream>
#include<fstream>

int main() {
	open();

	getSym();
	grammer();
	optimizeCode();
	translate();

	close();
}
