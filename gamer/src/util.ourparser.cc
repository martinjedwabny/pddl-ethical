// ***********************************************************
// 
//  Book:       Heuristic Search
// 
//  Authors:    S.Edelkamp, S.Schroedl
// 
//  See file README for information on using and copying 
//  this software.
// 
//  Project:    Mips - model checking integrated planning
//              system
// 
//  Module:     mips\src\util.tools.cc
//  Authors:    S.Edelkamp, M.Helmert
// 
// ***********************************************************


#include <cstdio>
//#include <algorithm>
//#include <iomanip>
#include <fstream>

#include <unistd.h>
//#include <stdio.h>
//#include <stdlib.h>
#include <vector>

using namespace std;

#include <util.ourparser.h>

Ourparser::Ourparser() {

}


bool Ourparser::shouldMipsGoOn(){ // write a "o" into .temp to go on
	std::ifstream von(".temp");
	if (!von) cerr << "the communication-file could not be opened" << endl;
	char zeichen;
	von.get(zeichen);
	von.close();
	return (int(zeichen) != 119);
}

void Ourparser::waitForEdit() {
cout << "mips is waiting for finishing edit..." << endl;
	while (! shouldMipsGoOn())
	{
		cerr << ".";
		//cout << ",";
		sleep(1);
	}
}

void Ourparser::tellScriptToGoOn() {
	std::ofstream raus(".temp");
	if (!raus) cerr << "the communication-file could not be opened" << endl;
	raus.put(char(119));
	raus.close();
}
