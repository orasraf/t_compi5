/*
 * genAssembly.hpp
 *
 *  Created on: Jan 22, 2018
 *      Author: user
 */

#ifndef GENASSEMBLY_HPP_
#define GENASSEMBLY_HPP_
#include <list>
#include <vector>
#include <iostream>
#include <cstddef>
#include <string>
#include "registerPool.hpp"
#include "external_file.hpp"

void freeTempReg(Register r);
Register getTempReg(list<string> regs);
pair<Register,bool> chooseReg(list<string> regs, Exp* exp);
void unchooseReg(Register r, bool flag);
Register loadPlace_allocs(Node_ptr exp_p);
void addRegs(Register r1, Register r2);
void subRegs(Register r1, Register r2);
void mulRegs(Register r1, Register r2);
void divRegs(Register r1, Register r2);
string loadword(Register r, int offset);
void createPrinters();
void reStoreFreeRegisters(list<Register> usedRegs);
void storeFreeRegisters(int init_offset);
void addReturnAssembly();


#endif /* GENASSEMBLY_HPP_ */
