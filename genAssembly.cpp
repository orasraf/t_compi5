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


//#define demit(x) (cb.emit(string(x) + "\t\t#" + __FILE__ + ":" + my_fucking_itoa(__LINE__ ) ))
#define demit(x) (cb.emit(x ))
#define dregAlloc() (rp.regAlloc(__LINE__))
using namespace std;

Register loadPlace_allocs(Node_ptr exp_p){
	Register r;
	if(!exp_p->isRegAllocated()){
		r = dregAlloc();
		demit(loadword(r,(exp_p)->getPlace()));
		//demit("ERROR : loadPlace_allocs");
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
	demit("subu $sp,$sp,4");
	demit("sw " + name + ",($sp)");
	return rp.regSpecName(name);
}

void freeTempReg(Register r){
	demit("lw " +r.getName() + ", ($sp)");
	demit("addu $sp, $sp, 4");
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
			r = dregAlloc();
		}
		demit(loadword(r,(exp)->getPlace()));
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
		demit("addu " + r1.getName() + ", " +  r1.getName() + ", " + r2.getName());
		rp.regRelease(r2);
}

void subRegs(Register r1, Register r2){
	demit("subu " + r1.getName() + ", " +  r1.getName() + ", " + r2.getName());
	rp.regRelease(r2);
}

void mulRegs(Register r1, Register r2){
	demit("mul " + r1.getName() + ", " +  r1.getName() + ", " + r2.getName());
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
		r3 = dregAlloc();
	}
	demit("li " + r3.getName() + ", 0");
	int origbp = demit("bne " + r2.getName() + ", " + r3.getName() + ", ");
	demit("la $a0, zero");
	demit("li $v0, 4");
	demit("syscall");
	demit("li $v0, 10");
	demit("syscall");
	string bp = cb.genLabel();
	demit("div " + r1.getName() + ", " +  r1.getName() + ", " + r2.getName());
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
	demit("printi:");
	demit("lw $a0, 4($fp)");
	demit("li $v0, 1");
	demit("syscall");
	demit("jr $ra");
	demit("print:");
	demit("lw $a0, 4($fp)");
	demit("li $v0, 4");
	demit("syscall");
	demit("jr $ra");
	return;
}

void storeFreeRegisters(int init_offset){
	list<Register> usedRegs = rp.getUsedRegs();
	for(list<Register>::iterator i = usedRegs.begin() ; i != usedRegs.end() ; i++){
		string os = my_fucking_itoa(init_offset,false);
		string os_toPrint = (os=="0")?"":os;
		demit(string("sw $") + my_fucking_itoa(i->getNum()+8,false) + ", " +os_toPrint +"($sp)");
		init_offset-=4;
	}
}

void reStoreFreeRegisters(list<Register> usedRegs){
	int init_offset = 0; //RegisterPool::REGISTERS_NUM * 4;
	for(list<Register>::reverse_iterator ri = usedRegs.rbegin() ; ri != usedRegs.rend() ; ri++){
		string os = my_fucking_itoa(init_offset,true);
		string os_toPrint = (os=="0")?"":os;
		demit(string("lw $") + my_fucking_itoa(ri->getNum()+8,false) + ", " +os_toPrint +"($sp)");
		init_offset-=4;
	}
}

void addReturnAssembly(){
	demit("move $sp, $fp");
	demit("jr $ra");
}

void genArrayAssignmentLoop(string array_size , string lhs_offset , string rhs_offset){
	Register osReg = dregAlloc();
	Register valReg = dregAlloc();
	string r1 = osReg.getName();
	string r2 = valReg.getName();
	array_size = my_fucking_itoa( atoi(array_size.c_str()) * 4 );
	cb.emit("#===START ARRAY ASSIGNMENT");
	demit("li " + r1 + ", -" + array_size);				//li r1 , -arrSize					r1 := -arrSize
	string loopLabel = cb.genLabel();
	demit("addu " + r1 + ", 4");							//loopLabel:addu r1 , 1				r1 := r1 + 1
    demit("addu " + r1 + ", " + r1 + ", $fp"); 			//addu r1 , r1 , $fp				r1 := r1 + $fp
	demit("addu " + r1 + ", " + r1 + ", " + rhs_offset);	//subu r1 , r1 , rhs_offset			r1 := r1 - rhs_offset
	demit("lw " + r2 + ", (" + r1 + ")");					//lw r2 , (r1)						r2 := (r1)
	demit("subu " + r1 + ", " + r1 + ", " + rhs_offset );	//addu r1 , r1 , rhs_offset			r1 := r1 + rhs_offset
	demit("addu " + r1 + ", " + r1 + ", " + lhs_offset );	//subu r1 , r1 , lhs_offset			r1 := r1 - lhs_offset
	demit("sw " + r2 + ", (" + r1 + ")");					//sw r2 , (r1)						(r1) := r2
	demit("subu " + r1 + ", " + r1 + ", " + lhs_offset );	//addu r1 , r1 , lhs_offset			r1 := r1 + lhs_offset
    demit("subu " + r1 + ", " + r1 + ", $fp"); 			//subu r1 , r1 , $fp				r1 := r1 - $fp
	demit("bne " + r1 + ", 0," + loopLabel);				//bne r1 , 0 , loopLabel			if r1 = 0 then pc := loopLable
	cb.emit("#===END ARRAY ASSIGNMENT");
	rp.regRelease(osReg);
	rp.regRelease(valReg);
}

void initArray(string arrType, string arrSize , string arrOffSet){
	Register osReg = dregAlloc();
	string r1 = osReg.getName();
	arrSize = my_fucking_itoa( atoi(arrSize.c_str()) * 4 );

	demit("li " + r1 + ", -" + arrSize);				//li r1 , -arrSize					r1 := -arrSize
	string loopLabel = cb.genLabel();
	demit("addu " + r1 + ", 4");							//loopLabel:addu r1 , 1				r1 := r1 + 1
    demit("addu " + r1 + ", " + r1 + ", $fp"); 			//addu r1 , r1 , $fp				r1 := r1 + $fp
	demit("addu " + r1 + ", " + r1 + ", " + arrOffSet);	//subu r1 , r1 , rhs_offset			r1 := r1 - arrOffSet
	demit(string("sw ") + "$0" + ", (" + r1 + ")");
	demit("subu " + r1 + ", " + r1 + ", " + arrOffSet );	//addu r1 , r1 , lhs_offset			r1 := r1 + arrOffSet
    demit("subu " + r1 + ", " + r1 + ", $fp"); 			//subu r1 , r1 , $fp				r1 := r1 - $fp
	demit("bne " + r1 + ", 0," + loopLabel);				//bne r1 , 0 , loopLabel			if r1 = 0 then pc := loopLable
	rp.regRelease(osReg);
}











