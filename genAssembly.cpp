/*
 * genAssembly.cpp
 *
 *  Created on: Jan 22, 2018
 *      Author: user
 */

#include "genAssembly.hpp"
#include "registerPool.hpp"
#include "bp.hpp"
//#include "parser.h"
#include <string>

extern RegisterPool rp;
extern CodeBuffer&  cb;

using namespace std;

Register loadPlace_allocs(Node_ptr exp_p){
	Register r;
	if(!exp_p->isRegAllocated()){
		r = rp.regAlloc();
		cb.emit(loadword(r,(exp_p)->getPlace()));
		//cb.emit("ERROR : loadPlace_allocs");
	} else {
		r = exp_p->getReg();
	}
	return r;
}
Register getTempReg(list<string> regs){
	string name = rp.getNotThese(regs);
	if (name=="NONE"){
		cout << "ERROR when looking for a free register!" << endl;
		name="$25";
	}
	cb.emit("subu $sp,$sp,4");
	cb.emit("sw " + name + ",($sp)");
	return rp.regSpecName(name);
}

void freeTempReg(Register r){
	cb.emit("lw " +r.getName() + ", ($sp)");
	cb.emit("addu $sp, $sp, 4");
	return;
}

pair<Register,bool> chooseReg(list<string> regs, Exp* exp){
	Register r;
	bool flag=false;
	if (exp->isRegAllocated()){
		r = exp->getReg();
	} else {
		if (rp.areAllUsed()){
			flag=true;
			list<string> l;
			r = getTempReg(l);
		} else {
			r = rp.regAlloc();
		}
		cb.emit(loadword(r,(exp)->getPlace()));
	}
	return make_pair(r,flag);
}

void unchooseReg(Register r, bool flag){
	if (flag){
		freeTempReg(r);
	} else {
		rp.regRelease(r);
	}
}


void addRegs(Register r1, Register r2){
		cb.emit("addu " + r1.getName() + ", " +  r1.getName() + ", " + r2.getName());
		rp.regRelease(r2);
}

void subRegs(Register r1, Register r2){
	cb.emit("subu " + r1.getName() + ", " +  r1.getName() + ", " + r2.getName());
	rp.regRelease(r2);
}

void mulRegs(Register r1, Register r2){
	cb.emit("mul " + r1.getName() + ", " +  r1.getName() + ", " + r2.getName());
	rp.regRelease(r2);
}

void divRegs(Register r1, Register r2){
	bool swapped = false;
	Register r3;
	if (rp.areAllUsed()){
		list<string> regs;
		regs.push_front(r2.getName());
		regs.push_front(r1.getName());
		swapped=true;
		r3 = getTempReg(regs);
	}else{
		r3 = rp.regAlloc();
	}
	cb.emit("li " + r3.getName() + ", 0");
	int origbp = cb.emit("bne " + r2.getName() + ", " + r3.getName() + ", ");
	cb.emit("la $a0, zero");
	cb.emit("li $v0, 4");
	cb.emit("syscall");
	cb.emit("li $v0, 10");
	cb.emit("syscall");
	string bp = cb.genLabel();
	cb.emit("div " + r1.getName() + ", " +  r1.getName() + ", " + r2.getName());
	cb.bpatch(cb.makelist(origbp),bp);
	if (swapped){
		freeTempReg(r3);
	} else {
		rp.regRelease(r3);
	}
	rp.regRelease(r2);
}

string loadword(Register r, int offset){
	string sign = "";
	if(offset > 0 ){
		sign = "-";
	}
	return "lw " + r.getName() + ", "+sign + my_fucking_itoa(offset,true)+"($fp)" ;
}

void createPrinters(){
	cb.emit("printi:");
	cb.emit("lw $a0, 4($fp)");
	cb.emit("li $v0, 1");
	cb.emit("syscall");
	cb.emit("jr $ra");
	cb.emit("print:");
	cb.emit("lw $a0, 4($fp)");
	cb.emit("li $v0, 4");
	cb.emit("syscall");
	cb.emit("jr $ra");
	return;
}

void storeFreeRegisters(int init_offset){
	list<Register> usedRegs = rp.getUsedRegs();
	for(list<Register>::iterator i = usedRegs.begin() ; i != usedRegs.end() ; i++){
		string os = my_fucking_itoa(init_offset,false);
		string os_toPrint = (os=="0")?"":os;
		cb.emit(string("sw $") + my_fucking_itoa(i->getNum()+8,false) + ", " +os_toPrint +"($sp)");
		init_offset-=4;
	}
}

void reStoreFreeRegisters(list<Register> usedRegs){
	int init_offset = 0; //RegisterPool::REGISTERS_NUM * 4;
	for(list<Register>::reverse_iterator ri = usedRegs.rbegin() ; ri != usedRegs.rend() ; ri++){
		string os = my_fucking_itoa(init_offset,true);
		string os_toPrint = (os=="0")?"":os;
		cb.emit(string("lw $") + my_fucking_itoa(ri->getNum()+8,false) + ", " +os_toPrint +"($sp)");
		init_offset-=4;
	}
}

void addReturnAssembly(){
	cb.emit("move $sp, $fp");
	cb.emit("jr $ra");
}

void genArrayAssignmentLoop(string array_size , string lhs_offset , string rhs_offset){
	Register osReg = rp.regAlloc();
	Register valReg = rp.regAlloc();
	string r1 = osReg.getName();
	string r2 = valReg.getName();
	array_size = my_fucking_itoa( atoi(array_size.c_str()) * 4 );
	cb.emit("li " + r1 + ", -" + array_size);				//li r1 , -arrSize					r1 := -arrSize
	string loopLabel = cb.genLabel();
	cb.emit("addu " + r1 + ", 4");							//loopLabel:addu r1 , 1				r1 := r1 + 1
    cb.emit("addu " + r1 + ", " + r1 + ", $fp"); 			//addu r1 , r1 , $fp				r1 := r1 + $fp
	cb.emit("subu " + r1 + ", " + r1 + ", " + rhs_offset);	//subu r1 , r1 , rhs_offset			r1 := r1 - rhs_offset
	cb.emit("lw " + r2 + ", (" + r1 + ")");					//lw r2 , (r1)						r2 := (r1)
	cb.emit("addu " + r1 + ", " + r1 + ", " + rhs_offset );	//addu r1 , r1 , rhs_offset			r1 := r1 + rhs_offset
	cb.emit("subu " + r1 + ", " + r1 + ", " + lhs_offset );	//subu r1 , r1 , lhs_offset			r1 := r1 - lhs_offset
	cb.emit("sw " + r2 + ", (" + r1 + ")");					//sw r2 , (r1)						(r1) := r2
	cb.emit("addu " + r1 + ", " + r1 + ", " + lhs_offset );	//addu r1 , r1 , lhs_offset			r1 := r1 + lhs_offset
    cb.emit("subu " + r1 + ", " + r1 + ", $fp"); 			//subu r1 , r1 , $fp				r1 := r1 - $fp
	cb.emit("bne " + r1 + ", 0," + loopLabel);				//bne r1 , 0 , loopLabel			if r1 = 0 then pc := loopLable
	rp.regRelease(osReg);
	rp.regRelease(valReg);
}

void initArray(string arrType, string arrSize , string arrOffSet){
	Register osReg = rp.regAlloc();
	string r1 = osReg.getName();
	arrSize = my_fucking_itoa( atoi(arrSize.c_str()) * 4 );

	cb.emit("li " + r1 + ", -" + arrSize);				//li r1 , -arrSize					r1 := -arrSize
	string loopLabel = cb.genLabel();
	cb.emit("addu " + r1 + ", 4");							//loopLabel:addu r1 , 1				r1 := r1 + 1
    cb.emit("addu " + r1 + ", " + r1 + ", $fp"); 			//addu r1 , r1 , $fp				r1 := r1 + $fp
	cb.emit("subu " + r1 + ", " + r1 + ", " + arrOffSet);	//subu r1 , r1 , rhs_offset			r1 := r1 - arrOffSet
	cb.emit(string("sw ") + "$0" + ", (" + r1 + ")");
	cb.emit("addu " + r1 + ", " + r1 + ", " + arrOffSet );	//addu r1 , r1 , lhs_offset			r1 := r1 + arrOffSet
    cb.emit("subu " + r1 + ", " + r1 + ", $fp"); 			//subu r1 , r1 , $fp				r1 := r1 - $fp
	cb.emit("bne " + r1 + ", 0," + loopLabel);				//bne r1 , 0 , loopLabel			if r1 = 0 then pc := loopLable
	rp.regRelease(osReg);
}











