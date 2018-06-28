/*
 * registersPool.cpp
 *
 *  Created on: Jan 22, 2018
 *      Author: user
 */
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <cstddef>
#include <string>
#include <list>
#include <sstream>
#include "registerPool.hpp"
#include <fstream>




using namespace std;

int RegisterPool::count = 0 ;
RegisterPool* RegisterPool::singletone = NULL ;


int dbz(int k){
	return k;
}

int divByZero(){
	return dbz(0);
}

Register::Register(){
	this->num=-1;
	this->name="";
	this->allocated=false;
}

Register::Register(int numa, string namea, bool allocateda){
	this->num=numa;
	this->name=namea;
	this->allocated=allocateda;
}

int Register::getNum(){
	return this->num;
}

void Register::setNum(int a){
	this->num=a;
}

string Register::getName(){
	return this->name;
}

void Register::setName(string s){
	this->name=s;
}

bool Register::getAllocated(){
	return this->allocated;
}

void Register::setAllocated(bool a){
	this->allocated=a;
}

string my_fucking_itoa(int i,bool ignore_sign){
	string ret = "";
	if(i==0){
		return "0";
	}
	int factor = 1;
	if(i < 0){
		factor = -1;
		i = -i;
	}
	while(i > 0){
		int new_i = i / 10 ;
		int mod_i = i % 10 ;
		ret= string(1,(char)((char) mod_i + '0')) + ret;
		i = new_i;
	}
	if(ignore_sign == false && factor == -1){
		return "-"+ret;
	}
	return ret;
}

RegisterPool::RegisterPool(){
	for (int i=0; i< REGISTERS_NUM; i++){
		string regName="$";
		string num = my_fucking_itoa(i+8);
		Register a = Register(i,regName+num,false);
		//cout << regName + num << endl;
		this->registers[i]=a;
		debug[i] = 0;
	}
}

Register RegisterPool::regSpecNum(int num){
	return this->registers[num];
}

Register RegisterPool::regSpecName(string name){
	for (int i=0; i<REGISTERS_NUM;i++){
		if (this->registers[i].getName()==name){
			return this->registers[i];
		}
	}
	int rc = 1/divByZero();
	exit(rc);
	return Register();
}

RegisterPool RegisterPool::getInstance(){
	if(count == 0 ){
		singletone = new RegisterPool();
		count++;
	}
	return *singletone;
}
Register RegisterPool::regAlloc(){
	for (int i=0; i<REGISTERS_NUM; i++){
		if (!(this->registers[i].getAllocated())){
			this->registers[i].setAllocated(true);
			return this->registers[i];
		}
	}

	cout << "your reg allocation is crap. seeya!" << endl;

	//int rc = 1/divByZero();
	printStack();
	exit(0);
}
Register RegisterPool::regAlloc(int line){
	for (int i=0; i<REGISTERS_NUM; i++){
		if (!(this->registers[i].getAllocated())){
			this->registers[i].setAllocated(true);
			debug[i] = line;
			return this->registers[i];
		}
	}

	cout << "your reg allocation is crap. seeya!" << endl;

	//int rc = 1/divByZero();
	printStack();
	exit(0);
}

void RegisterPool::regRelease(Register r){
	debug[r.getNum()] = 0;
	this->registers[r.getNum()].setAllocated(false);
}

bool RegisterPool::areAllUsed(){
	return false;
	for (int i=0; i<REGISTERS_NUM; i++){
		if (registers[i].getAllocated()==false){
			return false;
		}
	}
	return true;
}

void RegisterPool::printStack(){

	std::fstream fs ;
	fs.open("reg.txt" , std::fstream::out | std::fstream::trunc);
	for(int i = 0 ; i < 18 ; i++){
		if(debug[i]!=0 || registers[i].getAllocated()){
			fs << "Register " << i << " : " << debug[i] << " : " <<  registers[i].getAllocated() <<   endl;
		}
	}
	fs.close();
}

string RegisterPool::getNotThese(list<string> regs){
	list<string> all;
	for (int i=0; i<REGISTERS_NUM; i++){
		all.push_back(registers[i].getName());
	}
	for (int i=0; i<regs.size(); i++){
		all.remove(*(regs.begin()));
	}
	if (all.empty()){
		return "NONE";
	}
	return all.front();
}

list<Register> RegisterPool::getUsedRegs(){
	list<Register> usedRegs;
	for (int i=0; i<REGISTERS_NUM; i++){
		if (this->registers[i].getAllocated()){

			usedRegs.push_back(this->registers[i]);
		}
	}
	return usedRegs;
}

void RegisterPool::freeAllRegs(){
	for (int i=0; i<REGISTERS_NUM; i++){
		registers[i].setAllocated(false);
	}
}

void RegisterPool::reAllocateRegsList(list<Register> regs){
	for(list<Register>::iterator i = regs.begin() ;  i != regs.end() ; i++){
		registers[i->getNum()].setAllocated(true);
	}
}


