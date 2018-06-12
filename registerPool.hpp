/*
Z * registerPool.hpp
 *
 *  Created on: Jan 22, 2018
 *      Author: user
 */

#ifndef REGISTERPOOL_HPP_
#define REGISTERPOOL_HPP_

#include <iostream>
#include <cstddef>
#include <string>
#include <stdlib.h>
#include <list>
#include <vector>
#include <sstream>
#include <algorithm>


using namespace std;

string my_fucking_itoa(int i,bool ignore_sign);

class Register{

	 int num;
	 string name;
	 bool allocated;
public:
	 Register();
	 Register(int num, string name, bool allocated);
	 int getNum();
	 void setNum(int a);
	 string getName();
	 void setName(string s);
	 bool getAllocated();
	 void setAllocated(bool a);

};

class RegisterPool{
	static int count ;
	static RegisterPool* singletone ;
	Register registers[18];

public :
	static RegisterPool getInstance();
	static const int REGISTERS_NUM = 18;
	RegisterPool();
	Register regSpecNum(int num);
	Register regSpecName(string name);
	Register regAlloc();
	void regRelease(Register r);
	bool areAllUsed();
	string getNotThese(list<string> regs);
	list<Register> getUsedRegs();
	void freeAllRegs();
	void reAllocateRegsList(list<Register> regs);
};




#endif /* REGISTERPOOL_HPP_ */
