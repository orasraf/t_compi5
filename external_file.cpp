#include <iostream>
#include <cstddef>
#include <string>
#include <stdlib.h>
//#include "yystype.h"
#include "output.hpp"
#include <list>
#include <vector>
#include "external_file.hpp"
#include <sstream>
#include <algorithm>
#include "genAssembly.hpp"
#include "bp.hpp"


#define YYSTYPE Node*
//#define STACK_OFFSET_UPDATE  "80"
//#define STACK_OFFSET_UPDATE_INT  80
using namespace std;


extern CodeBuffer&  cb;
extern RegisterPool rp;
int stringsCounter = 0;

int newStringLable(){
	return stringsCounter++;
}
//extern PlaceStack sp;

int k ;
void traverse(Node_ptr node){
	int sons = node->getNumberOfSons();
	if( node == NULL){
		return;
	}
	for(int s = 0; s < sons ; s++){
		traverse(node->getSon(s));
	}
	////cout << k++ << " " ;

}
void evalExp(Node_ptr exp_p){

	Exp* exp = static_cast<Exp*>(exp_p);
	if(!exp->isBool){
		return;
	}
//	cb.emit("#EVAL EXP START");
	//cb.emit("#TEST >> exp is boolean");
	string trueLine = cb.next();
	Register r = rp.regAlloc();
	exp->setReg(r);
	cb.emit("li " + r.getName() + ", 1");
	int nextLineTrue = cb.emit("j ");
	cb.bpatch(exp->truelist,trueLine);
	string falseLine = cb.next();
	cb.emit("li " + r.getName() + ", 0");
	int nextLineFalse = cb.emit("j ");

	cb.bpatch(exp->falselist,falseLine);
	string psudo_quad = cb.next();
	cb.bpatch(cb.makelist(nextLineTrue),psudo_quad);
	cb.bpatch(cb.makelist(nextLineFalse),psudo_quad);
//	cb.emit("#EVAL EXP END");
}

string getStackOffset(Ter* id){
	int address = st.getName(id->val)->offSet*4;
	string negative=my_fucking_itoa(address,true);
	string sign = address>0?"-":"";
	string offset = sign + negative ;
	return offset;
}
int getNumberOfArgs(ExpList* expList_p){
	bool last = false;
	int numOfSons = 0;
	int numOfArgs = 0;

	while(!last){
			numOfSons = expList_p->getNumberOfSons();
			if(numOfSons == 1 ){
				last = true;
			} else if(numOfSons == 3 ){
				expList_p = (ExpList*)expList_p->getSon(2); // the rest of the list
			} else {
				////cout << "num of sons for ExpList is not 3 or 1." << endl;
				exit(0);
			}
			numOfArgs++;
	}
		return numOfArgs;
}

int pushArgs(ExpList* expList_p,int regsListSize){
	int numOfArgs = getNumberOfArgs(expList_p);// excluding args' registers
	int numOfArgs_ret = numOfArgs;
	int actualRegsListSize  = regsListSize -  numOfArgs;
	bool last = false;
	int numOfSons = 0;

	Exp* exp;
	while(!last){
		numOfSons = expList_p->getNumberOfSons();
		if(numOfSons == 1 ){
			last = true;
			exp =  (Exp*)expList_p->getSon(0);
		} else if(numOfSons == 3 ){
			exp = (Exp*)expList_p->getSon(0);
			expList_p = (ExpList*)expList_p->getSon(2); // the rest of the list
		} else {
			////cout << "num of sons for ExpList is not 3 or 1." << endl;
			exit(0);
		}

//		int place = exp->getPlace();
//
//		Register r = rp.regAlloc();
//		cb.emit(loadword(r,place));
		Register r = exp->getReg();

		cb.emit(string("sw ")  + r.getName() + ", " +
				my_fucking_itoa(-4*(--numOfArgs)-(actualRegsListSize+1+1)*4,false) + "($sp)"); //regs + fp + ra

		exp->releaseReg();

	}
	return numOfArgs_ret;
}

void closeFunctionScope(){
	st.closeFunctionScope();
}

void openScope(){
	st.enterScope();
}

void closeScope(){

	st.exitScope();
}

void whileLabel(Node_ptr exp_p, Node_ptr statement_p, Node_ptr marker){
	Exp* exp = static_cast<Exp*>(exp_p);
	Statement* statement = static_cast<Statement*>(statement_p);
	M* m = static_cast<M*>(marker);
	cb.bpatch(cb.merge(exp->falselist,statement->breakList), m->quad);
}

void LoadIFLines(Node_ptr exp_p){
//
//	if(!exp_p1->isRegAllocated()){
//
//	}
//	if(!exp_p2->isRegAllocated()){
//
//	}
//	if(exp_p1->isRegAllocated() && exp_p2->isRegAllocated()){
//		int trueline = cb.emit("beq " + r.getName() + ", 1, ");
//		int falseline = cb.emit("j ");
//		exp->truelist.push_back(trueline);
//		exp->falselist.push_back(falseline);
//	}
//
//	return;
	Exp* exp = static_cast<Exp*>(exp_p);

	if ( true or exp->isBool){

		Register r;

		if (!exp->isRegAllocated()){
//			cb.emit("#Reg is no allocated in LoadIFLines");
			r = rp.regAlloc();

			if (exp->isPlaceSet()){
				int place = 0-exp->getPlace();
				cb.emit("lw " + r.getName() + ", " + my_fucking_itoa(place,false) + "($fp)");
			} else {
				if (exp->value=="true"){
					cb.emit("li " + r.getName() + ", 1");
				} else if(exp->value=="false") {
					cb.emit("li " + r.getName() + ", 0");
				} else {
//					cb.emit("#ERROR un initialized value");
				}
			}

		} else {
			r = exp->getReg();
			evalExp(exp);
		}
		int trueline = cb.emit("beq " + r.getName() + ", 1, ");
		int falseline = cb.emit("j ");
		exp->truelist.push_back(trueline);
		exp->falselist.push_back(falseline);

		rp.regRelease(r);

	}
}

types stringToType(string a){
	if (a=="INT"){
		return INT_t;
	} else if (a=="BYTE"){
		return BYTE_t;
	} else if (a=="BOOL"){
		return BOOL_t;
	} else if (a=="STRING"){
		return STRING_t;
	} else if (a=="VOID"){
		return VOID_t;
	}
	return UNDEF_t;
}

string typeToString(types a){
	if (a==INT_t){
		return "INT";
	} else if (a==BYTE_t){
		return "BYTE";
	} else if (a==BOOL_t){
		return "BOOL";
	} else if (a==STRING_t){
		return "STRING";
	} else if (a==VOID_t){
		return "VOID";
	}
	return "UNDEF";
}
// ============ implementing Ter
	Ter::Ter(string val,string kind):val(val),kind(kind){}
	string Ter::getVal(){
		return val;
	}


//===========implementing Type
Type::Type(string a, int lineno):Node(1){
		Ter* name = new Ter(a,a);
		types temp = stringToType(a);
		this->val=temp;
		this->str=a;
		setSon(0,name);
	}


//=======implementing Node


Node::Node():numberOfSons(0),sons(NULL),reg_allocated(false),nextList(NULL),db_name(0),place_allocated(false){}
Node::Node(int numSons):numberOfSons(numSons),reg_allocated(false),nextList(NULL),db_name(0),place_allocated(false){
		sons = new Node_ptr[numSons];
	}
Node::~Node(){
		for(int i; i< numberOfSons; i++){
			delete sons[i];
		}
		delete[] this->sons;
	}
void Node::setNumberOfSons(int num){
		if(num<0 || num > 15){
			st.set_prints(false); exit(0);
		}
		sons = new Node_ptr[num];
	}
int Node::getNumberOfSons(){
		return numberOfSons;
	}

int Node::getPlace(){
	if(place_allocated==false){
		//TODO: DELETE THIS BELOEW
//		cb.emit("#ERROR: place was not allocated");
	}
	return place;
}
void Node::setPlace(int a){
	place =a;
	place_allocated = true;
}
bool Node::isPlaceSet(){
	return place_allocated;
}
void Node::setSon(int idx, Node_ptr son_p){
		if(idx<0 || idx >= numberOfSons){
			st.set_prints(false); exit(0);
		}
		sons[idx] = son_p;
		//toEmitAppend(son_p->gettoEmit());
		if(son_p->place_allocated == true){
			if(place_allocated == true){
				place_allocated = false;

			} else {
				place_allocated = true;
				place = son_p->place;
			}
		}
	}
Node_ptr Node::getSon(int idx){
		if(idx<0 || idx >= numberOfSons){
			return NULL;
		}
		return sons[idx];
	}


// ======================== IMPLEMENT CALL =====
	Call::Call(Node_ptr id_p, int lineno):Node(3){
				UsedRegsList = rp.getUsedRegs();
				//need to check the id in the symballs table and also the number and types of varuables or not..
				Ter* id = static_cast<Ter*>(id_p); // need to to the check
				if (st.isNameDefined(id->val)){
					const Name* myName = st.getName(id->val);
					if (myName->type!="func"){
						output::errorUndefFunc(lineno, id->val);
						st.set_prints(false); exit(0);
					}
					if (myName->numerOfParams!=0){
						vector<string> paramList;
						for(int i=0; i < myName->numerOfParams ; i++){
							paramList.push_back(myName->parameters[i].first);
						}
						output::errorPrototypeMismatch(lineno,id->val,paramList);
						st.set_prints(false); exit(0);
					}
					type = stringToType(myName->returnType);
				} else {
					output::errorUndefFunc(lineno, id->val);
					st.set_prints(false); exit(0);
				}
				if  (id->val=="print" || id->val=="printi"){
					output::errorMismatch(lineno);
					st.set_prints(false); exit(0);
				}
				Ter* lparen = new Ter("(","LPAREN");
				Ter* rparen = new Ter(")","RPAREN");
				setSon(0,id);
				setSon(1,lparen);
				setSon(2,rparen);


				string func_name = id->getVal();
				cb.emit("sw $fp, ($sp)");
				cb.emit("sw $ra , -4($sp)");	//store ret address
				storeFreeRegisters(-8);
				rp.freeAllRegs();
				//string stack_offset_update = my_fucking_itoa((1+1+RegisterPool::REGISTERS_NUM)*4,false);

				cb.emit(string("li $8, ") + my_fucking_itoa((UsedRegsList.size() +1+1)*4 ,true));
				cb.emit("subu $sp, $sp, $8"); //update the stack pointer SP
				cb.emit("move $fp, $sp");
				cb.emit(string("jal ")+ "_" + func_name); // this is to assure that no funcion can have

				cb.emit("li $8, 4");
				cb.emit("addu $sp, $sp, $8");						// a label that is emmited from CB.
				reStoreFreeRegisters(UsedRegsList);
				cb.emit("lw $ra , "+my_fucking_itoa(UsedRegsList.size()*4 ,false)+"($sp)");	//restore ret address
				cb.emit("lw $fp "+my_fucking_itoa(UsedRegsList.size()*4 +4 ,false)+"($sp)");
				rp.reAllocateRegsList(UsedRegsList);
				Register temp_r = rp.regAlloc();
				cb.emit("li "+ temp_r.getName() + ", " +  my_fucking_itoa((UsedRegsList.size() +1)*4  ,true));
				cb.emit("addu $sp, $sp, " + temp_r.getName());
				rp.regRelease(temp_r);
//				int stack_offset = sp.newTemp_and_emit(Register(-1,"$v0",true));
//				setPlace(stack_offset);

				Register ret_r  = rp.regAlloc();
				cb.emit("move " + ret_r.getName() + ", $v0");
				setReg(ret_r);

				db_name = 15;

				//sp.clear();

			}
		Call::Call(Node_ptr id_p, Node_ptr expList_p, int lineno):Node(4){

			Ter* id = static_cast<Ter*>(id_p); // need to to the check
			Ter* lparen = new Ter("(","LPAREN");
			Ter* rparen = new Ter(")","RPAREN");
			ExpList* expList = static_cast<ExpList*>(expList_p);
			vector<string> expTypes = expList->myList;
			if (st.isNameDefined(id->val)){
				const Name* myName = st.getName(id->val);
				if (myName->type!="func"){
					output::errorUndefFunc(lineno, id->val);
					st.set_prints(false); exit(0);
				}
				vector<string>::iterator expIt = expTypes.begin();
				vector<string> paramList;
				bool errType = false;
				if (myName->numerOfParams != expTypes.size()){
					errType=true;
				}
				for(int i=0 ; i< myName->numerOfParams; i++){
					paramList.push_back(myName->parameters[i].first);
					if (myName->parameters[i].first!=(*expIt)){
						if (!(myName->parameters[i].first=="INT" && (*expIt)=="BYTE")){
							errType=true;
						}
					}
					if (expIt!=expTypes.end()){
						expIt++;
					}
				}
				if (errType){
					output::errorPrototypeMismatch(lineno,id->val,paramList);
					st.set_prints(false); exit(0);
				}
				this->type=stringToType(myName->returnType);
			} else {
				output::errorUndefFunc(lineno, id->val);
				st.set_prints(false); exit(0);
			}

			setSon(0,id);
			setSon(1,lparen);
			setSon(2,expList);
			setSon(3,rparen);
			db_name =2;
			string func_name = id->getVal();
			//unique to this Call with args list


			int numOfArgs = pushArgs((ExpList*)expList_p,rp.getUsedRegs().size());

			UsedRegsList = rp.getUsedRegs();

			//
			//skip over the places
			Register skip_r = rp.regAlloc();
//			cb.emit(string("li ") + skip_r.getName() +", " + my_fucking_itoa(sp.getStackOffset(),true));
//			cb.emit("subu $sp, $sp," + skip_r.getName() );
			rp.regRelease(skip_r);
			//store fp, ra
			cb.emit("sw $fp, ($sp)");
			cb.emit("sw $ra , -4($sp)");	//store ret address
			storeFreeRegisters(-8);
			rp.freeAllRegs();
			//string stack_offset_update = my_fucking_itoa((1+1+RegisterPool::REGISTERS_NUM)*4,false);

			//update the sp to saved regs + args num

			cb.emit(string("li $8, ") + my_fucking_itoa((numOfArgs)*4  + (UsedRegsList.size() +1+1)*4  ,false));
			cb.emit("subu $sp, $sp, $8"); //update the stack pointer SP
			cb.emit("move $fp, $sp");
			// jump to func
			string underScore = "_";
			if(func_name == "print" || func_name == "printi"){
				underScore ="";
			}
			cb.emit(string("jal ")+ underScore + func_name); // this is to assure that no funcion can have
			//skip back on args

			cb.emit(string("li $8, ") + my_fucking_itoa((1+numOfArgs)*4,true));

			cb.emit("addu $sp, $sp, $8");
			//restore regs
			reStoreFreeRegisters(UsedRegsList);
			cb.emit("lw $ra , "+my_fucking_itoa(UsedRegsList.size()*4 ,false)+"($sp)");	//restore ret address
			cb.emit("lw $fp, "+my_fucking_itoa(UsedRegsList.size()*4 +4 ,false)+"($sp)");
			rp.reAllocateRegsList(UsedRegsList);
			Register temp_r = rp.regAlloc();
			cb.emit("li "+ temp_r.getName() + ", " +
					my_fucking_itoa((UsedRegsList.size() +1)*4  ,true));
			cb.emit("addu $sp, $sp, " + temp_r.getName());
			rp.regRelease(temp_r);
			Register ret_r = rp.regAlloc();
			cb.emit("move " + ret_r.getName() + ", $v0");
			setReg(ret_r);
			//int stack_offset = sp.newTemp_and_emit(Register(-1,"$v0",true));

			//setPlace(stack_offset);

			//sp.clear();
		}


//============ implementing Exp




	Register Node::getReg() {
		if(reg_allocated == false){
			////cout << "illeagal register access. shutting down" << endl;
			//exit(0);
			reg = rp.regAlloc();
		}
		return reg;
	}
	bool Node::isRegAllocated(){
		return reg_allocated;
	}
	void Node::setReg(Register r){
		reg = r;
		reg_allocated = true;
	}
	void Node::releaseReg(){
		rp.regRelease(reg);
		reg_allocated = false;
	}
	void Exp::EqFunc(types* type, string* val, Exp& b, Exp& c,int ln){
		if (b.type==BOOL_t || c.type==BOOL_t || b.type==STRING_t || c.type==STRING_t || b.type==VOID_t || c.type==VOID_t){
			output::errorMismatch(ln);
			st.set_prints(false); exit(0);
		}
		*type=BOOL_t;

		Register r1 = loadPlace_allocs(&b);
		Register r2 = loadPlace_allocs(&c);

		if (b.value.length()>0 && c.value.length()>0){
			if (b.value!=c.value){
				*val="false";
			} else {
				*val="true";
			}
		}

		int trueline = cb.emit("beq " + r1.getName() + ", " + r2.getName() + " ");
		truelist.push_back(trueline);
		int falseline = cb.emit("j ");
		falselist.push_back(falseline);
		rp.regRelease(r1);
		rp.regRelease(r2);
	}

	void Exp::NeqFunc(types* type, string* val, Exp& b, Exp& c, int ln){
		if (b.type==BOOL_t || c.type==BOOL_t || b.type==STRING_t || c.type==STRING_t || b.type==VOID_t || c.type==VOID_t){
			output::errorMismatch(ln);
			st.set_prints(false); exit(0);
		}

		Register r1 = loadPlace_allocs(&b);
		Register r2 = loadPlace_allocs(&c);

		*type=BOOL_t;
		if (b.value.length()>0 && c.value.length()>0){
			if (b.value!=c.value){
				*val="true";
			} else {
				*val="false";
			}
		}

		int trueline = cb.emit("bne " + r1.getName() + ", " + r2.getName() + " ");
		truelist.push_back(trueline);
		int falseline = cb.emit("j ");
		falselist.push_back(falseline);
		rp.regRelease(r1);
		rp.regRelease(r2);

	}

	void Exp::SeqFunc(types* type, string* val, string op, Exp& b, Exp& c, int ln){
		if (b.type==BOOL_t || c.type==BOOL_t || b.type==STRING_t || c.type==STRING_t || b.type==VOID_t || c.type==VOID_t){
			output::errorMismatch(ln);
			st.set_prints(false); exit(0);
		}
		*type=BOOL_t;

		Register r1 = loadPlace_allocs(&b);
		Register r2 = loadPlace_allocs(&c);

		if (b.value.length()>0 && c.value.length()>0){
			if (atoi(b.value.c_str())<=atoi(c.value.c_str())){
				*val="true";
			} else {
				*val="false";
			}
		}

		int trueLine = 0;
		if(op=="<="){
			 trueLine = cb.emit("ble " + r1.getName() + ", " + r2.getName() + " " );
		} else if (op==">="){
			trueLine = cb.emit("bge " + r2.getName() + ", " + r1.getName() + " " );
		} else {
			////cout << "bad arguments to smlFunc. exiting" << endl;
			exit(0);
		}
		truelist.push_back(trueLine);
		int falseLine = cb.emit("j ");
		falselist.push_back(falseLine);
		rp.regRelease(r1);
		rp.regRelease(r2);
	}

	void Exp::SmlFunc(types* type, string* val,string op ,  Exp& b, Exp& c, int ln){
		if (b.type==BOOL_t || c.type==BOOL_t || b.type==STRING_t || c.type==STRING_t || b.type==VOID_t || c.type==VOID_t){
			output::errorMismatch(ln);
			st.set_prints(false); exit(0);
		}
		*type=BOOL_t;

		Register r1 = loadPlace_allocs(&b);
		Register r2 = loadPlace_allocs(&c);

		int trueLine = 0;
		if(op=="<"){
			 trueLine = cb.emit("blt " + r1.getName() + ", " + r2.getName() + " " );
		} else if (op==">"){
			trueLine = cb.emit("bgt " + r1.getName() + ", " + r2.getName() + " " );
		} else {
			////cout << "bad arguments to smlFunc. exiting" << endl;
			exit(0);
		}
		truelist.push_back(trueLine);

		int falseLine = cb.emit("j ");
		falselist.push_back(falseLine);
//		////cout << "debug falselist" << endl;
//		////cout << falselist.size() << endl;
		rp.regRelease(r1);
		rp.regRelease(r2);
	}

	void Exp::MathFunc(types* type, string* val, Exp& b, Exp& c, int ln, string op){
		if (b.type==BOOL_t || c.type==BOOL_t || b.type==STRING_t || c.type==STRING_t || c.type==VOID_t || b.type==VOID_t){
			output::errorMismatch(ln);
			st.set_prints(false); exit(0);
		}
		if (b.type!=c.type){
			*type=INT_t;
		} else {
			*type=b.type;
		}
		stringstream ss;

		if (true || b.value.length()>0 || c.value.length()>0){

			Register r1 = loadPlace_allocs(&b);
			Register r2 = loadPlace_allocs(&c);


			if (op=="+"){
				ss << atoi(b.value.c_str())+atoi(c.value.c_str());
								*val = "";// ss.str();
								//toEmitAppend(addRegs(r1,r2));
								addRegs(r1,r2);
								setReg(r1);

			} else if (op=="-"){
				ss << atoi(b.value.c_str())-atoi(c.value.c_str());
								*val = "";//ss.str();
								//toEmitAppend(subRegs(r1,r2));
								subRegs(r1,r2);
								setReg(r1);

			} else if (op=="*"){
				ss << atoi(b.value.c_str())*atoi(c.value.c_str());
								*val = "";//ss.str();
								//toEmitAppend( mulRegs(r1,r2));
								mulRegs(r1,r2);
								setReg(r1);

			} else if (op=="/"){

				//ss << atoi(b.value.c_str())/atoi(c.value.c_str());
				//toEmitAppend(divRegs(r1,r2));
				divRegs(r1,r2);
				setReg(r1);
				*val = "";//ss.str();

			}
			if (*type==BYTE_t){
				string msg = "and " + getReg().getName() + ", " + getReg().getName() + ", " + "255";
				//toEmitAppend(msg);
				cb.emit(msg);
			}
			rp.regRelease(r2);
		}
	}

	void Exp::AndFunc(types* type, string* val,  Exp& b,  Exp& c, int ln, M* m ){
		if (b.type!=BOOL_t || c.type!=BOOL_t){
			output::errorMismatch(ln);
			st.set_prints(false); exit(0);
		}
		*type=BOOL_t;
		if (b.value.length()>0 && c.value.length()>0){
			if (b.value=="true" && c.value=="true"){
				*val="true";
			} else {
				*val="false";
			}
		}

	    cb.bpatch(b.truelist,m->quad);
	    truelist = c.truelist;
	    falselist = cb.merge(b.falselist,c.falselist);
	    if (b.isRegAllocated()){
	    	rp.regRelease(b.getReg());
	    }
	    if (c.isRegAllocated()){
	    	rp.regRelease(c.getReg());
	    }
	    isBool = true;
	}

	void Exp::OrFunc(types* type, string* val,  Exp& b,  Exp& c, int ln , M* m ){
		if (b.type!=BOOL_t || c.type!=BOOL_t){
			output::errorMismatch(ln);
			st.set_prints(false); exit(0);
		}
		*type=BOOL_t;
		if (b.value.length()>0 && c.value.length()>0){
			if (b.value=="false" && c.value=="false"){
				*val="false";
			} else {
				*val="false";
			}
		}

		cb.bpatch(b.falselist,m->quad);
		truelist = cb.merge(b.truelist,c.truelist);
		falselist = c.falselist;
//		////cout << "debug falselist after merge" << endl;
//		////cout << falselist.size() << endl;
		if (b.isRegAllocated()){
			rp.regRelease(b.getReg());
		}
		if (c.isRegAllocated()){
			rp.regRelease(c.getReg());
		}
		isBool = true;
	}



	Exp::Exp(string op , Node_ptr leftNode , Node_ptr rightNode, int lineno ,Node_ptr marker):
		Node(3),type(UNDEF_t),value(""),lineno(lineno),isBool(false){

		Exp* left = static_cast<Exp*>(leftNode);
		Exp* right = static_cast<Exp*>(rightNode);
		M* m = static_cast<M*>(marker);
		// Shai's shit
		if (left->type!=right->type){
			if (!((left->type==BYTE_t && right->type==INT_t) || (left->type==INT_t && right->type==BYTE_t))){
				output::errorMismatch(lineno);
				st.set_prints(false); exit(0);
			}
		}
		if (left->type==UNDEF_t || right->type==UNDEF_t){
			output::errorMismatch(lineno);
			st.set_prints(false); exit(0);
		}
		if (left->type==VOID_t || right->type==VOID_t){
			output::errorMismatch(lineno);
			st.set_prints(false); exit(0);
		}

		types myType=UNDEF_t;
		string myVal="";
		Op* temp_op = new Op(op);

		this->setSon(0,left);
		this->setSon(1,temp_op);
		this->setSon(2,right);
		if (op=="=="){
			isBool =true;
			EqFunc(&myType,&myVal,*left,*right,lineno);
		} else if (op=="!="){
			isBool =true;
			NeqFunc(&myType,&myVal,*left,*right,lineno);
		} else if (op=="<="){
			isBool =true;
			SeqFunc(&myType,&myVal,op,*left,*right,lineno);
		} else if (op==">="){
			isBool =true;
			SeqFunc(&myType,&myVal,op,*right,*left,lineno);
		} else if (op=="<"){
			isBool =true;
			SmlFunc(&myType,&myVal,op,*left,*right,lineno);
		} else if (op==">"){
			isBool =true;
			SmlFunc(&myType,&myVal,op,*left,*right,lineno);
		} else if (op=="+"){
			MathFunc(&myType,&myVal,*left,*right,lineno,"+");
		} else if (op=="-"){
			MathFunc(&myType,&myVal,*left,*right,lineno,"-");
		} else if (op=="*"){
			MathFunc(&myType,&myVal,*left,*right,lineno,"*");
		} else if (op=="/"){
			MathFunc(&myType,&myVal,*left,*right,lineno,"/");
		} else if (op=="AND"){
			if(m == NULL ){
				////cout << "marker before ANDFunc is null " << endl;
				exit(0);
			}
			isBool =true;
			AndFunc(&myType,&myVal,*left,*right,lineno,m);
		} else if (op=="OR"){
			if(m == NULL ){
				////cout << "marker before OrFunc is null " << endl;
				exit(0);
			}
			isBool =true;
			OrFunc(&myType,&myVal,*left,*right,lineno,m);
		}
		this->type=myType;
		this->value=myVal;


	}

	Exp::Exp(Node_ptr onlySon, int lineno):Node(2),type(UNDEF_t),value(""),lineno(lineno),isBool(false){
		Exp* son = static_cast<Exp*>(onlySon);

		if (son->type!=BOOL_t){
			output::errorMismatch(lineno);
			st.set_prints(false); exit(0);
		}
		this->type=BOOL_t;

		if (son->value=="true"){
			this->value="false";
		}
		if (son->value=="false"){
			this->value="true";
		}

		if (son->isBool){
			list<string> l;
			Register r;
			bool flag = false;
			pair<Register,bool> temp = chooseReg(l,son);
			r = temp.first;
			flag = temp.second;
			int trueline = cb.emit("beq " + r.getName() + ", 1, ");
			int falseline = cb.emit("j ");
			son->truelist.push_back(trueline);
			son->falselist.push_back(falseline);
			unchooseReg(r,flag);
		}
		//rp.regRelease(son->getReg());
		Op* not_op = new Op("NOT");
		truelist = son->falselist;
		falselist = son->truelist;

		this->setSon(0,not_op);
		this->setSon(1,son);

	}

	Exp::Exp(string opa, Node_ptr onlySon, string opb, int lineno):Node(3),type(UNDEF_t),
			value(""),lineno(lineno),isBool(false){
		Exp* son = static_cast<Exp*>(onlySon);

		this->type=son->type;
		this->value=son->value;

		Ter* lparen = new Ter("(","LPAREN");
		Ter* rparen = new Ter(")","RPAREN");

		if (son->isBool){
			isBool = true;
		}

		truelist = son->truelist;
		falselist = son->falselist;

		this->setSon(0,lparen);
		this->setSon(1,son);
		this->setSon(2,rparen);
		if (son->isRegAllocated()){
			setReg(son->getReg());
		}
	}


	Exp::Exp(string op, Node_ptr onlySon, int lineno):Node(1),type(UNDEF_t),value(""),lineno(lineno),isBool(false){
		string toEmit="";

		if (op=="Call"){
			Call* son = static_cast<Call*>(onlySon);
			db_name = son->db_name;

			this->type=son->type;
			if (son->isRegAllocated()){

				setReg(son->getReg());
			}
			//setPlace(sp.newTemp_and_emit(Register(-1,"$v0",false)));
//			Register r = rp.regAlloc();
//			string emit = "lw " + r.getName() + ", " + "<take_value_from_stack>";
//			//toEmitAppend(emit);
//			setReg(r);
		} else {
			Ter* son = static_cast<Ter*>(onlySon);
			if (op=="INT"){
				this->type=INT_t;
				Register r = rp.regAlloc();
				string emit = "li " + r.getName() + ", " + son->val;
//				////cout << "debug: emit in INT  : " << endl;
//				////cout << "					 : " + emit << endl;
				//toEmitAppend(emit);
				cb.emit(emit);
//				setPlace(sp.newTemp_and_emit(r));
//				rp.regRelease(r);
				setReg(r);
			} else if (op=="TRUE" || op=="FALSE"){
				this->type=BOOL_t;
				//Register r = rp.regAlloc();
				if (op=="TRUE"){
					int trueline = cb.emit("j ");
					truelist = cb.makelist(trueline);
				} else {
					int falseLine = cb.emit("j ");
					falselist = cb.makelist(falseLine);
				}
				//setReg(r);
				isBool = true;
			} else if (op=="B"){
				if(atoi(son->val.c_str())>255){
					output::errorByteTooLarge(lineno,son->val);
					st.set_prints(false); exit(0);
				}
				Register r = rp.regAlloc();
				string emit = "li " + r.getName() + ", " + son->val;
//				////cout << "debug: emit in BYTE : " << endl;
//				////cout << "					 : " + emit << endl;
				//toEmitAppend(emit);
				cb.emit(emit);
//				setPlace(sp.newTemp_and_emit(r));
//				rp.regRelease(r);
				setReg(r);

				this->type=BYTE_t;
			} else if (op=="ID"){
				if (!(st.isNameDefined(son->val))){
					output::errorUndef(lineno,son->val);
					st.set_prints(false); exit(0);
				} else {
					if( true or st.getName(son->val)->offSet >= 0) {

//						Register r = rp.regAlloc();
//						string emit = "li " + r.getName() + ", " + "<need_to_fill_from_stack>";
//						son->//toEmitAppend(emit);
//						setReg(r);
						string offset = getStackOffset(son) ;
						Register r = rp.regAlloc();
//						////cout << "debug type ===== : ==== " << endl;
//						////cout << "(son->val) = " << son->val << endl;
//						////cout << "st.getName(son->val)->type = " << st.getName(son->val)->type << endl;
						if (st.getName(son->val)->type=="B"){

							//toEmitAppend("lw " + r.getName() + ", " + offset + "($fp)");
							cb.emit("lw " + r.getName() + ", " + offset + "($fp)");
						}
						else {
							//toEmitAppend("lb "+ r.getName() + ", " + offset + "($fp)");
							cb.emit("lw " + r.getName() + ", " + offset + "($fp)");
						}
						setReg(r);
					} else {

						setPlace(st.getName(son->val)->offSet * 4 );




					}

					Name* temp =st.getNoneConstName(son->val);
					this->type = stringToType(temp->type);
					if (type==BOOL_t){
						isBool = true;
					}
				}
				//this will tell the program that the place of this exp is already allocated bc its ID
				//setPlace(st.getName(son->val)->offSet);
			} else if (op=="STRING"){
				this->type=STRING_t;
				string lable = string("_str_") + my_fucking_itoa(newStringLable(),false);
				cb.emitData(lable + ": "+ ".asciiz " + son->getVal());
				setReg(rp.regAlloc());
				cb.emit("la " + getReg().getName() + ", " + lable);
			}
			this->value=son->getVal();


			//string///Lable = lable;

		}
		this->setSon(0,onlySon);

	}

//===========implementing M ========
	M::M():Node(0){
		db_name = 55;
		quad  = cb.next();
		//////cout << "M::M() quad [" << quad << "]" << endl;
	}

//===========implementing N ========
	N::N():Node(0){
		db_name = 1;
		int gotoLine = cb.emit("j ");
		nextList = cb.makelist(gotoLine);
	}

//============implemeting ExpList ========
ExpList::ExpList(Node_ptr exp_p, int lineno):Node(1){
		db_name = 22;
		Exp* exp = static_cast<Exp*>(exp_p);
		this->size=1;
		this->type=exp->type;
		string temp = typeToString(exp->type);
		myList.insert(myList.begin(),temp);
		setSon(0,exp);

	}
ExpList::ExpList(Node_ptr exp_p, Node_ptr explist_p, int lineno):Node(3){
	db_name = 23;
		Exp* exp = static_cast<Exp*>(exp_p);
		ExpList* explist = static_cast<ExpList*>(explist_p);
		this->size=explist->size+1;
		this->type=UNDEF_t;
		string temp = typeToString(exp->type);
		myList = explist->myList;
		myList.insert(myList.begin(),temp);
		Ter* comma = new Ter(",","COMMA");
		setSon(0,exp);
		setSon(1,comma);
		setSon(2,explist);
	}

// implementing StatementIF
StatementIF::StatementIF(Node_ptr exp_p, Node_ptr statement_p, int lineno, Node_ptr marker ):Node(3){
	db_name = 30;
	CheckNoBreak(statement_p,lineno);
	Exp* exp = static_cast<Exp*>(exp_p);
	Statement* statement = static_cast<Statement*>(statement_p);
	M* m = static_cast<M*>(marker);
	setSon(0,exp);
	setSon(1,statement);
	setSon(2,marker);

	cb.bpatch(exp->truelist,m->quad);
	nextList = cb.merge(exp->falselist,statement->nextList);
	breakList = statement->breakList;
//	////cout << "debug StatementIF exp->falselist "<<exp->falselist.size() << endl;
//	////cout << "debug StatementIF nextlist "<<nextList.size() << endl;

	//cb.bpatch(exp->falselist,cb.next());

}

//========= implemening Statement
Statement::Statement(string ter, int lineno):Node(2){
		is_break=false;
		is_return=false;
		is_emidiate_return=false;
		if (ter=="BREAK"){
			this->is_break=true;
			this->ln=lineno;
			this->breakList.push_back(cb.emit("j "));
		}
		if (ter=="RETURN"){
			is_emidiate_return=true;
			this->is_return=true;
			pair<types,int> a = make_pair(VOID_t,lineno);
			return_types.insert(return_types.begin(),a);
			cb.emit("move $sp, $fp");
			cb.emit("jr $ra");


		}
		Ter* term = new Ter(ter,ter);
		Ter* sc = new Ter(";","SC");
		setSon(0,term);
		setSon(1,sc);
		this->ln=lineno;
	}
Statement::Statement(Node_ptr call_p, int lineno):Node(2){
	db_name = 41;
		is_break=false;
		is_return=false;
		is_emidiate_return=false;
		Call* call = static_cast<Call*>(call_p);
		Ter* sc = new Ter(";","SC");
		setSon(0,call);
		setSon(1,sc);
		if(isRegAllocated()){
			call->releaseReg();
		}



		this->ln=lineno;
	}
Statement::Statement(string kind, Node_ptr b_p, int lineno):Node(3){
	db_name = 42;
		is_return=false;
		is_break=false;
		is_emidiate_return=false;
		Ter* a;
		Ter* c;
		Node_ptr b;
		if (kind=="STATEMENTS"){
			a = new Ter("{","LBRACE");
			Statements* b2 = static_cast<Statements*>(b_p);
			if (b2->is_break){
				this->is_break = true;
				this->ln = b2->ln;
			}
			if (b2->is_return){
				this->is_return=true;
				if (!(b2->return_types.size()==0)){
					return_types.insert(return_types.end(), b2->return_types.begin(), b2->return_types.end());
				}
			}
			b = b2;
			c = new Ter("}","RBRACE");
			breakList = b2->breakList;
		} else {
			a = new Ter("return","RETURN");
			Exp *b2 = static_cast<Exp*>(b_p);
			c = new Ter(";","SC");
			this->is_return = true;
			pair<types,int> a = make_pair(b2->type,lineno);
			this->return_types.insert(return_types.begin(),a);
			b = b2;


			// no params ret value

			Register r = loadPlace_allocs(b2);

			cb.emit(string("move $v0") +", " + r.getName());
			cb.emit("move $sp, $fp");
			cb.emit("jr $ra");
			rp.regRelease(r);
//			reStoreFreeRegisters();
//			Register temp_r = rp.regAlloc();
//			cb.emit("li "+ temp_r.getName() + ", " + STACK_OFFSET_UPDATE);
//			cb.emit("addu $sp, $sp, " + temp_r.getName());
//			cb.emit("lw $ra , -8($sp)");	//restore ret address
//			cb.emit("lw $fp -4($sp)");
//			rp.regRelease(temp_r);



		}
		nextList = b_p->nextList;
//		////cout << "b_p->nextList S - > L" <<  endl;
//		for(int i=0 ; i <nextList.size() ; i ++){
//			////cout << "b_p->nextList S - > L" << b_p->nextList[i] << endl;
//		}
		setSon(0,a);
		setSon(1,b);
		setSon(2,c);
		this->ln=lineno;
	}
Statement::Statement(Node_ptr type_p, Node_ptr id_p, int lineno):Node(3),ln(0){
	db_name = 43;
		is_return=false;
		is_break=false;
		is_emidiate_return=false;
		Type* type = static_cast<Type*>(type_p);
		Ter* id = static_cast<Ter*>(id_p);
		Ter* sc = new Ter(";","SC");
		if (st.isNameDefined(id->val)){
			output::errorDef(lineno,id->val);
			st.set_prints(false); exit(0);
		} else {
			st.addNameable(Name(id->val, type->str));

			//Not pushing anything there since id was not given a value
		}
		setSon(0,type);
		setSon(1,id);
		setSon(2,sc);
		//toEmitAppend("subu $sp, $sp, 4");

		Register r = rp.regAlloc();
		cb.emit("li " + r.getName() + ", 0");
		cb.emit("sw " + r.getName() + ", ($sp)");
		cb.emit("subu $sp, $sp, 4");
		rp.regRelease(r);
		//this->ln=lineno;
	}
Statement::Statement(string op, Node_ptr id_p, Node_ptr exp_p, int lineno):Node(4){
	db_name = 44;
		is_return=false;
		is_break=false;
		is_emidiate_return=false;
		Ter* id = static_cast<Ter*>(id_p);
		Ter* ass = new Ter("=",op);
		Exp* exp = static_cast<Exp*>(exp_p);
		if (st.isNameDefined(id->val)){
			const Name* temp = st.getName(id->val);
			types tempType = stringToType(temp->type);
			if(temp->type == "func"){
				output::errorUndef(lineno , id->val);
				st.set_prints(false); exit(0);
			}
			if (tempType!=exp->type){
				if (!(tempType==INT_t && exp->type==BYTE_t)){
					output::errorMismatch(lineno);
					st.set_prints(false); exit(0);
				}
			}
			if (tempType==BYTE_t){
				if (atoi(exp->value.c_str())>255){
					output::errorByteTooLarge(lineno,exp->value);
					st.set_prints(false); exit(0);
				}
			}
		} else {
			output::errorUndef(lineno,id->val);
			st.set_prints(false); exit(0);
		}
		Ter* sc = new Ter(";","SC");
		string offset = getStackOffset(id);

		//TODO: make sure the addressing here works correctly
		setSon(0,id);
		setSon(1,ass);
		setSon(2,exp);
		setSon(3,sc);

		//toEmitAppend("sw " + exp->getReg().getName() + ", " + offset + "($fp)");

//		Register r = rp.regAlloc();
//		cb.emit(loadword(r,_place));
//		cb.emit("sw " + r.getName() + ", "exp->getReg() + offset + "($fp)");

		Register r;
		if (exp->isRegAllocated()){
			r = exp->getReg();
			cb.emit("sw " + r.getName() + ", " + offset + "($fp)");
		} else if (exp->value=="true" or exp->value=="false"){
			r = rp.regAlloc(); // TODO: see what you can do about this
			if (exp->value=="true"){
				cb.emit("li " + r.getName() + ", 1");
			} else if (exp->value=="false"){
				cb.emit("li " + r.getName() + ", 0");
			}
			cb.emit("sw " + r.getName() + ", " + offset + "($fp)");
		} else {
			r = rp.regAlloc(); // TODO: check this as well
			string truelabel = cb.next();
			cb.emit("li " + r.getName() + ", 1");
			int endtrue = cb.emit("j ");
			string falselabel = cb.next();
			cb.emit("li " + r.getName() + ", 0");
			int endfalse = cb.emit("j ");
			string endline = cb.next();
			cb.emit("sw " + r.getName() + ", " + offset + "($fp)");
			cb.bpatch(exp->truelist,truelabel);
			cb.bpatch(exp->falselist,falselabel);
			vector<int> temp;
			temp.push_back(endtrue);
			temp.push_back(endfalse);
			cb.bpatch(temp, endline);
		}

		rp.regRelease(r);

		this->ln=lineno;
		//sp.clear();
	}

Statement::Statement(Node_ptr if_p, string noneed, int lineno):Node(5){
	db_name = 45;
//	////cout << "Statement::Statement(Node_ptr if_p, string noneed, int lineno):Node(5){" << endl;
	StatementIF* statementif = static_cast<StatementIF*>(if_p);
	is_return=false;
	is_break=false;
	is_emidiate_return=false;
	Statement* statement = static_cast<Statement*>(statementif->getSon(1));
	Exp* exp = static_cast<Exp*>(statementif->getSon(0));
	if (statement->is_return){
		this->is_return=true;
		if (!(statement->return_types.size()==0)){
			return_types.insert(return_types.end(),statement->return_types.begin(),statement->return_types.end());
		}
	}
	Ter* start = new Ter("IF","IF");
	Ter* lparen = new Ter("(","LPAREN");
	Ter* rparen = new Ter(")","RPAREN");
	//statement->db_name="the else part";
	setSon(0,start);
	setSon(1,lparen);
	setSon(2,exp);
	setSon(3,rparen);
	setSon(4,statement);
	if (statement->is_break){
		this->is_break=true;
		this->ln=statement->ln;
	}
	nextList = statementif->nextList;
	breakList = statementif->breakList;
//	////cout << "nextlist S - > S_IF" <<  endl;
//	for(int i=0 ; i <nextList.size() ; i ++){
//		////cout << "nextlist S - > S_IF" << nextList[i] << endl;
//	}

}

Statement::Statement(Node_ptr exp_p, Node_ptr statement_p, string op, int lineno, Node_ptr marker1, Node_ptr marker2):Node(5){
//		////cout << "in Statement::Statement(Node_ptr exp_p, Node_ptr statement_p, string op, int lineno):Node(5){ " << endl;
		is_return=false;
		is_break=false;
		is_emidiate_return=false;
		Statement* statement = static_cast<Statement*>(statement_p);
//		if (op=="IF"){
//			if (statement->is_break){
//				output::errorUnexpectedBreak(statement->ln);
//				st.set_prints(false); exit(0);
//			}
//		}
		if (statement->is_return){
			this->is_return=true;
			if (!(statement->return_types.size()==0)){
				return_types.insert(return_types.end(),statement->return_types.begin(),statement->return_types.end());
			}
		}
		Exp* exp = static_cast<Exp*>(exp_p);
//		if (exp->type!=BOOL_t){
//			output::errorMismatch(lineno);
//			st.set_prints(false); exit(0);
//		}
		Ter* start = new Ter(op,op);
		Ter* lparen = new Ter("(","LPAREN");
		Ter* rparen = new Ter(")","RPAREN");
		M* m1 = static_cast<M*>(marker1);
		M* m2 = static_cast<M*>(marker2);
		setSon(0,start);
		setSon(1,lparen);
		setSon(2,exp);
		setSon(3,rparen);
		setSon(4,statement);


		cb.bpatch(statement->nextList, m1->quad);
		cb.bpatch(exp->truelist, m2->quad);
		statement->nextList=exp->falselist;
		cb.emit("j "+ m1->quad);
		breakList = statement->breakList;

		if (statement->is_break){
			this->is_break=true;
			this->ln=statement->ln;
		}
	}
Statement::Statement(string op1, Node_ptr exp_p, Node_ptr caselist_p, string op2, int lineno,
		Node_ptr N_marker, Node_ptr marker):Node(8){
		is_return=false;
		is_break=false;
		is_emidiate_return=false;
		CaseList* caselist = static_cast<CaseList*>(caselist_p);
		Ter* sw = new Ter(op1,op2);
		Ter* lparen = new Ter("(","LPAREN");
		Exp* exp = static_cast<Exp*>(exp_p);
//		if (!(exp->type==INT_t || exp->type==BYTE_t)){
//			output::errorMismatch(lineno);
//			st.set_prints(false); exit(0);
//		}
		Ter* rparen = new Ter(")","RPAREN");
		Ter* lbrace = new Ter("{","LBRACE");
		Ter* rbrace = new Ter("}","RBRACE");
		Ter* sc = new Ter(";","SC");
		N* n = static_cast<N*>(N_marker);
		N* m = static_cast<N*>(marker);
		cb.bpatch(n->nextList,cb.next());
		int value;
		string quad;
		Register r1;
		bool stack1 = false;
		if (exp->isRegAllocated()){
			r1=exp->getReg();
		} else {
			if (rp.areAllUsed()){
				list<string> l;
				r1 = getTempReg(l);
				stack1 = true;
			} else {
				r1 = rp.regAlloc();
			}
			//cb.emit(string("li ")+ r1.getName() + ", " + my_fucking_itoa(exp->getPlace(),false));
		}
		while(!(caselist->value_list.empty())){
			value=caselist->value_list.back();
			caselist->value_list.pop_back();
			quad=caselist->quad_list.back();
			caselist->quad_list.pop_back();
			Register r2;
			bool stack2 = false;
			if (rp.areAllUsed()){
				list<string> l;
				l.push_front(r1.getName());
				r2= getTempReg(l);
				stack2 = true;
			} else {
				r2 = rp.regAlloc();
			}
			cb.emit("li "+ r2.getName() + ", " + my_fucking_itoa(value,false));
			cb.emit("beq " + r1.getName() + ", " + r2.getName() + ", " + quad);
			if (stack2){
				freeTempReg(r2);
			} else {
				rp.regRelease(r2);
			}
		}
		if (stack1){
			freeTempReg(r1);
		} else {
			rp.regRelease(r1);
		}
		cb.next();
		this->nextList=caselist->nextList;
		if (caselist->default_count>0){
			cb.emit(string("j ")+caselist->default_label);
		}
		cb.bpatch(cb.merge(m->nextList,caselist->breakList),cb.next());

		setSon(0,sw);
		setSon(1,lparen);
		setSon(2,exp);
		setSon(3,rparen);
		setSon(4,lbrace);
		setSon(5,caselist);
		setSon(6,rbrace);
		setSon(7,sc);
		this->ln=lineno;
	}
Statement::Statement(Node_ptr type_p, Node_ptr id_p, Node_ptr exp_p, int lineno):Node(5){
	db_name = 48;
		is_return=false;
		is_break=false;
		is_emidiate_return=false;
		Type* type = static_cast<Type*>(type_p);
		Exp* exp = static_cast<Exp*>(exp_p);
		Ter* id = static_cast<Ter*>(id_p);

		if (st.isNameDefined(id->val)){
			output::errorDef(lineno,id->val);
			st.set_prints(false); exit(0);
		} else {

			if (type->val!=exp->type){
				if (!((type->val==INT_t && exp->type==BYTE_t))){
					output::errorMismatch(lineno);
					st.set_prints(false); exit(0);
				}
			}
			st.addNameable(Name(id->val,type->str));
			const Name* temp = st.getName(id->val);
				if(temp==NULL){
				}
				types tempType = stringToType(temp->type);
			if (tempType==BYTE_t){
				if (atoi(exp->value.c_str())>255){
					output::errorByteTooLarge(lineno,exp->value);
					st.set_prints(false); exit(0);
				}
			}

		}
		Ter* ass = new Ter("=","ASSIGN");
		Ter* sc = new Ter(";","SC");
		string offset = getStackOffset(id);


		setSon(0,type);
		setSon(1,id);
		setSon(2,ass);
		setSon(3,exp);
		setSon(4,sc);
		//sp.clear();
		if(type->val==BOOL_t){
			//cb.emit("#TEST >> exp is boolean");
//			string trueLine = cb.next();
//			Register r = rp.regAlloc();
//			exp->setReg(r);
//			cb.emit("li " + r.getName() + ", 1");
//			int nextLineTrue = cb.emit("j ");
//			cb.bpatch(exp->truelist,trueLine);
//			string falseLine = cb.next();
//			cb.emit("li " + r.getName() + ", 0");
//			int nextLineFalse = cb.emit("j ");
//
//			cb.bpatch(exp->falselist,falseLine);
//			string psudo_quad = cb.next();
//			cb.bpatch(cb.makelist(nextLineTrue),psudo_quad);
//			cb.bpatch(cb.makelist(nextLineFalse),psudo_quad);
		} else {
			//cb.emit("#TEST >> exp is NOT boolean");
		}
		//toEmitAppend("sw " + exp->getReg().getName() + ", ($sp)");

		Register r;
		if (exp->isRegAllocated()){
			r = exp->getReg();
			cb.emit("sw " + r.getName() + ", ($sp)");
			cb.emit("subu $sp, $sp, 4");
		} else if (exp->value=="true" or exp->value=="false"){
			r = rp.regAlloc(); // TODO: see what you can do about this
			if (exp->value=="true"){
				cb.emit("li " + r.getName() + ", 1");
			} else if (exp->value=="false"){
				cb.emit("li " + r.getName() + ", 0");
			}
			cb.emit("sw " + r.getName() + ", ($sp)");
			cb.emit("subu $sp, $sp, 4");
		} else {
			r = rp.regAlloc(); // TODO: check this as well
			string truelabel = cb.next();
			cb.emit("li " + r.getName() + ", 1");
			int endtrue = cb.emit("j ");
			string falselabel = cb.next();
			cb.emit("li " + r.getName() + ", 0");
			int endfalse = cb.emit("j ");
			string endline = cb.next();
			cb.emit("sw " + r.getName() + ", ($sp)");
			cb.emit("subu $sp, $sp, 4");
			cb.bpatch(exp->truelist,truelabel);
			cb.bpatch(exp->falselist,falselabel);
			vector<int> temp;
			temp.push_back(endtrue);
			temp.push_back(endfalse);
			cb.bpatch(temp, endline);
		}

		rp.regRelease(r);
		this->ln=lineno;
	}
Statement::Statement(Node_ptr if_p, Node_ptr sb_p, int lineno, int twice,Node_ptr N_marker, Node_ptr M_marker)
:Node(7),ln(0){
	db_name = 49;
		CheckNoBreak(sb_p,lineno); //orig
		is_return=false;
		is_break=false;
		is_emidiate_return=false;
		StatementIF* statementif = static_cast<StatementIF*>(if_p);
		Statement* sta = static_cast<Statement*>(statementif->getSon(1));
		Statement* stb = static_cast<Statement*>(sb_p);

		if (sta->is_return){
			this->is_return=true;
			if (!(sta->return_types.size()==0)){
				return_types.insert(return_types.end(),sta->return_types.begin(),sta->return_types.end());
			}
		}
		if (stb->is_return){
			this->is_return=true;
			if (!(stb->return_types.size()==0)){
				return_types.insert(return_types.end(),stb->return_types.begin(),stb->return_types.end());
			}
		}
		Exp* exp = static_cast<Exp*>(statementif->getSon(0));
		Statement* st = static_cast<Statement*>(statementif->getSon(1));
		Ter* start = new Ter("IF","IF");
		Ter* lparen = new Ter("(","LPAREN");
		Ter* rparen = new Ter(")","RPAREN");
		Ter* end = new Ter("ELSE","ELSE");
		breakList = cb.merge(statementif->breakList,stb->breakList);

		/* for reference with statementIF sons
		setSon(0,exp);
		setSon(1,statement);
		setSon(2,marker);
		*/


		M* m2 = static_cast<M*>(M_marker);
		N* n = static_cast<N*>(N_marker);
		//already patched  B.truelist in StatementIF rule
		//db_name="the else part";
		cb.bpatch(exp->falselist,m2->quad);
//		////cout <<"cb.bpatch(exp->falselist,m2->quad);;"<<endl;
//		for(int i=0 ; i < exp->falselist.size() ; i++){
//			////cout << "exp->falselist " << exp->falselist[i] << endl;
//		}
		nextList = cb.merge(cb.merge(st->nextList,n->nextList),stb->nextList);
//		////cout <<"nextList = cb.merge(cb.merge(sta->nextList,n->nextList),stb->nextList);"<<endl;
//		for(int i=0 ; i < nextList.size() ; i++){
//			////cout << "nextlist " << nextList[i] << endl;
//		}

		setSon(0,start);
		setSon(1,lparen);
		setSon(2,exp);
		setSon(3,rparen);
		setSon(4,sta);
		setSon(5,end);
		setSon(6,stb);
	}


// ============= implementing Statements ========
//Statements::Statements(Node_ptr statement_p, int lineno,Node_ptr M_marker ):Node(1){
//		is_break=false;
//		is_return=false;
//		return_types = list<pair<types,int> >() ;
//		Statement* statement = static_cast<Statement*>(statement_p);
//		if (statement->is_break){
//			this->is_break=true;
//			this->ln=statement->ln;
//		}
//		if (statement->is_return){
//			this->is_return=true;
//			if (!(statement->return_types.size()==0)){
//				this->return_types.insert(this->return_types.end(), statement->return_types.begin(), statement->return_types.end());
//			}
//		}
//		M* m = static_cast<M*>(M_marker);
//		cb.bpatch(statement->nextList,m->quad);
//		nextList = statement->nextList;
//
//		setSon(0,statement);
//	}
Statements::Statements(Node_ptr statement_p, int lineno ):Node(1){
	db_name = 60;
		is_break=false;
		is_return=false;
		return_types = list<pair<types,int> >() ;
		Statement* statement = static_cast<Statement*>(statement_p);
		if (statement->is_break){
			this->is_break=true;
			this->ln=statement->ln;
		}
		if (statement->is_return){
			this->is_return=true;
			if (!(statement->return_types.size()==0)){
				this->return_types.insert(this->return_types.end(), statement->return_types.begin(), statement->return_types.end());
			}
		}

		nextList = statement->nextList;
		breakList = statement->breakList;
//		////cout << "nextlist L - > S" <<  endl;
//		for(int i ; i < nextList.size() ; i++){
//			////cout << "nextlist L - > S" << nextList[i] << endl;
//		}
		setSon(0,statement);
	}
Statements::Statements(Node_ptr statements_p, Node_ptr statement_p, int lineno, Node_ptr M_marker):Node(2){
//		////cout <<" Statements ======> Statements statement" << endl;
//		////cout<< "||| Statement_s nextListSize " << statements_p->nextList.size()<< endl;
//		if(statements_p->nextList.size() > 0){
//			////cout << statements_p->nextList[0] << endl;
//			////cout << "quad "<< static_cast<M*>(M_marker)->quad <<endl;
//		}
//		////cout<< "||| Statement nextListSize " << statement_p->nextList.size()<< endl;
//		if(statement_p->nextList.size() > 0){
//			////cout << statement_p->nextList[0] << endl;
//
//		}
//		////cout <<"END Statements ======> Statements statement END" << endl;
	db_name = 61;
		is_break=false;
		is_return=false;
		return_types = list<pair<types,int> >() ;
		Statements* statements = static_cast<Statements*>(statements_p);
		Statement* statement = static_cast<Statement*>(statement_p);
		M* m = static_cast<M*>(M_marker);
		if (statement->is_break){
			this->is_break=true;
			this->ln=statement->ln;
		} else if (statements->is_break){
		this->is_break=true;
		this->ln=statements->ln;
		}
		if (statement->is_return){
			this->is_return=true;
			if (!(statements->return_types.size()==0)){
				this->return_types.insert(return_types.end(),statements->return_types.begin(),statements->return_types.end());
			}
			if (!(statement->return_types.size()==0)){
				this->return_types.insert(return_types.end(),statement->return_types.begin(),statement->return_types.end());
			}
		}
//		////cout << "nextlist L - > L1 M S (statements)" << endl;
//		for(int i=0 ; i < statements->nextList.size() ; i++){
//			////cout << "nextlist L - > L1 M S (statements)" << statements->nextList[i] << endl;
//		}
		breakList = cb.merge(
				statements->breakList,
				statement->breakList);
//		////cout << "# nextList in L -> L <m> S size is " + my_fucking_itoa(statements->nextList.size(), false) <<endl;
//		for(vector<int>::iterator i = statements->nextList.begin() ; i!=statements->nextList.end();i++){
//			////cout<< "  " << *i << endl;
//		}
//		////cout << "m->quad : " << m->quad << endl;
		cb.bpatch(
				statements->nextList,
				m->quad);

		nextList = statement->nextList;
//		////cout <<"sanity check: nom of sons " << statement->getNumberOfSons()<<endl;
//		if(statement->getNumberOfSons()==5){
//			////cout <<"sanity check: " << statement->getSon(4)->db_name<<endl;
//		}

//		////cout << "nextlist L - > L1 M S" << endl;
//		for(int i=0 ; i < nextList.size() ; i++){
//			////cout << "nextlist L - > L1 M S" << nextList[i] << endl;
//		}
//		////cout << "M: " << m->quad << endl;
		//cb.printCodeBuffer();
		setSon(0,statements);
		setSon(1,statement);

	}



//================implementing FuncDecl
FuncDecl::FuncDecl(Node_ptr rettype_p, Node_ptr id_p, Node_ptr formals_p, Node_ptr statements_p, int lineno, Node_ptr M_marker):Node(8){
	db_name = 70;
	is_main = false;
		Statements* statements = static_cast<Statements*>(statements_p);
//		if (statements->is_break){
//			output::errorUnexpectedBreak(statements->ln);
//			st.set_prints(false); exit(0);
//		}

		RetType* rettype = static_cast<RetType*>(rettype_p);
//		if (statements->is_return){
//			if (((statements->return_type=="" || statements->return_type=="NOTVOID") && rettype->is_void) || (statements->return_type=="VOID" && !(rettype->is_void))){
//				output::errorMismatch(lineno);
//				st.set_prints(false); exit(0);
//			}
//		}
		if (statements->return_types.begin()!=statements->return_types.end()){
			for(list<pair<types,int> >::iterator it = statements->return_types.begin();
					it!=statements->return_types.end(); it++){
				if(it->first != rettype->type){
					if (!(it->first==BYTE_t && rettype->type==INT_t)){
						output::errorMismatch(it->second);
						st.set_prints(false); exit(0);
					}
				}
			}
		}

		Ter* id = static_cast<Ter*>(id_p);
//		st.addNameable(Name(id->val,"func")); //this isnt finished yet, make sure this is what needs to be passed
		Formals* formals = static_cast<Formals*>(formals_p);
		if (id->val=="main" && rettype->is_void && formals->is_empty){ // can i assume that main is written in lower case letters?
			this->is_main=true;
		}
//		Name* funcName= st.getNoneConstName(id->val);
//		funcName->update(formals->decl);
//		st.setFuncParams(formals->decl);
		Ter* lparen = new Ter("(","LPAREN");
		Ter* rparen = new Ter(")","RPAREN");
		Ter* lbrace = new Ter("{","LBRACE");
		Ter* rbrace = new Ter("}","RBRACE");
		M* m = static_cast<M*>(M_marker);
		//////cout << "debug statements->nextList.size(): " << statements->nextList.size() << endl;
		cb.bpatch(statements->nextList,m->quad);
		setSon(0,rettype);
		setSon(1,id);
		setSon(2,lparen);
		setSon(3,formals);
		setSon(4,rparen);
		setSon(5,lbrace);
		setSon(6,statements);
		setSon(7,rbrace);
		rp.freeAllRegs();

	}

// ========== implementing Funcs =====
Funcs::Funcs(Node_ptr funcdecl_p, Node_ptr funcs_p, int lineno):Node(2){
	db_name = 80;
		is_main=false;
		FuncDecl* funcdecl = static_cast<FuncDecl*>(funcdecl_p);
		Funcs* funcs = static_cast<Funcs*>(funcs_p);
		if (funcdecl->is_main || funcs->is_main){
			this->is_main=true;
		}
		setSon(0,funcdecl);
		setSon(1,funcs);
	}
Funcs::Funcs(int lineno):Node(){
	db_name = 81;
		is_main=false;
		//TODO: figure out if this is the way to handle epsilons, and if so, change the parser to match
	}

//==============implement Program
Program::Program(Node_ptr funcs_p, int lineno):Node(1){
	db_name = 90;
		Funcs* funcs = static_cast<Funcs*>(funcs_p);
		if (!(funcs->is_main)){
			output::errorMainMissing();
			st.set_prints(false); exit(0);
		}
		setSon(0,funcs);
		createPrinters();
		//////cout<< "==================" << endl;
		//////cout<< "Code in Code Buffer" << endl;
		cb.printCodeBuffer() ;
		cb.printDataBuffer();
		//traverse(this);
	}


//=================== imlemenintg RetType

RetType::RetType(Node_ptr type_p, int lineno):Node(1){

		is_void = false;
		Type* type = static_cast<Type*>(type_p);
		this->type = type->val;
		setSon(0,type);
	}
RetType::RetType(int lineno):Node(1){

		Ter* v = new Ter("VOID","VOID");
		this->is_void = true;
		this->type=VOID_t;
		setSon(0,v);
	}

//====== implementing Formals
Formals::Formals(Node_ptr formalslist_p, int lineno):Node(1){

		is_empty = false;
		FormalsList* formalslist = static_cast<FormalsList*>(formalslist_p);
		this->decl = formalslist->decl;
		setSon(0,formalslist);
	}
Formals::Formals(int lineno):Node(){
		//TODO: figure out if this is the way to handle epsilons, and if so, change the parser to match
		is_empty=true;
	}


//=============implementing FormalsList====
FormalsList::FormalsList(Node_ptr formaldecl_p, int lineno):Node(1){
		count_formals = 0;
		FormalDecl* formaldecl = static_cast<FormalDecl*>(formaldecl_p);
		this->decl.push_back(formaldecl->nameable);
		count_formals++;
		setSon(0,formaldecl);
	}
FormalsList::FormalsList(Node_ptr formaldecl_p, Node_ptr formalslist_p, int lineno):Node(3){
		count_formals = 0;
		FormalDecl* formaldecl = static_cast<FormalDecl*>(formaldecl_p);
		Ter* comma = new Ter(",","COMMA");
		FormalsList* formalslist = static_cast<FormalsList*>(formalslist_p);
		this->decl = formalslist->decl;
		this->decl.push_front(formaldecl->nameable);
		count_formals=formalslist->count_formals+1;
		setSon(0,formaldecl);
		setSon(1,comma);
		setSon(2,formalslist);
	}



//========== implementing FormalsDecl
FormalDecl::FormalDecl(Node_ptr type_p, Node_ptr id_p, int lineno):Node(2){
		Type* type = static_cast<Type*>(type_p);
		Ter* id = static_cast<Ter*>(id_p);
		if (st.isNameDefined(id->val)){
			output::errorDef(lineno, id->val);
			st.set_prints(false); exit(0);
		}
		this->nameable = pair<string,string>(type->str,id->val);
		setSon(0,type);
		setSon(1,id);
	}




//========== implementing CaseList

int maxilin(int ln1, int ln2){
	if (ln1>ln2){
		return ln1;
	} else {
		return ln2;
	}
}

CaseList::CaseList(Node_ptr casestatement_p, int lineno):Node(1){
		default_count=0;
		CaseStatement* casestatement = static_cast<CaseStatement*>(casestatement_p);
		this->default_count=casestatement->default_count;
		if (default_count){
			this->ln=casestatement->ln1;
			this->default_label = casestatement->default_label;
		} else {
			this->ln=-1;
		}
		list<string> ls;
		list<int> li;
		this->quad_list = ls;
		this->value_list = li;
		if (casestatement->default_count==0){
			this->quad_list.push_front(casestatement->quad);
			this->value_list.push_front(casestatement->value);
		}
		this->nextList=casestatement->nextList;
		breakList = casestatement->breakList;
		setSon(0,casestatement);
	}
CaseList::CaseList(Node_ptr caselist_p, Node_ptr casestatement_p, int lineno):Node(2){
		default_count=0;
		CaseList* caselist = static_cast<CaseList*>(caselist_p);
		CaseStatement* casestatement = static_cast<CaseStatement*>(casestatement_p);
		bool default_flag=false;
		this->default_count = casestatement->default_count + caselist->default_count;
		if (this->default_count>1){
			int temp = maxilin(casestatement->ln1,caselist->ln);
			output::errorTooManyDefaults(temp);
			st.set_prints(false); exit(0);
		} else {
			if (casestatement->ln1>0){
				this->ln = casestatement->ln1;
				this->default_label = casestatement->default_label;
				default_flag=true;
			} else if (caselist->ln>0){
				this->ln = caselist->ln;
				this->default_label=caselist->default_label;
			}
		}
		this->quad_list=caselist->quad_list;
		this->value_list=caselist->value_list;
		if (!(default_flag)){
			this->quad_list.push_front(casestatement->quad);
			this->value_list.push_front(casestatement->value);
		}
		this->nextList=cb.merge(caselist->nextList,casestatement->nextList);
		breakList = cb.merge(caselist->breakList, casestatement->breakList);

		setSon(0,caselist);
		setSon(1,casestatement);
	}
//


//==========implementing caseStatements
CaseStatement::CaseStatement(Node_ptr casedec_p, int lineno):Node(1){
		default_count=0;
		this->ln1=-1;
		CaseDec* casedec = static_cast<CaseDec*>(casedec_p);
		if(casedec->default_count!=0){
			this->default_count++;
			this->ln1=casedec->ln;
			this->default_label = casedec->default_label;
		}
		setSon(0,casedec);
	}
CaseStatement::CaseStatement(Node_ptr casedec_p, Node_ptr statements_p, int lineno , Node_ptr marker):Node(2){
		default_count=0;
		this->ln1=-1;
		CaseDec* casedec = static_cast<CaseDec*>(casedec_p);
		Statements* statements = static_cast<Statements*>(statements_p);
		if(casedec->default_count!=0){
			this->default_count++;
			this->ln1=casedec->ln;
			this->default_label = casedec->default_label;
		}
		M* m = static_cast<M*>(marker);
		this->quad=m->quad;
		if (casedec->numberOfSons>2){
			Ter* num = static_cast<Ter*>(casedec->getSon(1));
			this->value=atoi(num->getVal().c_str());
		}
		this->nextList=statements->nextList;
		breakList = statements->breakList;
		setSon(0,casedec);
		setSon(1,statements);
	}

//===========IMPLEMENTING CaseDec
CaseDec::CaseDec(int lineno):Node(2){
		default_count=0;
		Ter* deft = new Ter("default","DEFAULT");
		Ter* colon = new Ter("colon", "COLON");
		this->default_label = cb.next();
		this->default_count = 1;
		this->ln = lineno;
		setSon(0,deft);
		setSon(1,colon);
	}
CaseDec::CaseDec(Node_ptr num_p, int lineno):Node(3){
		default_count=0;
		this->ln=-1;
		Ter* num = static_cast<Ter*>(num_p);
		Ter* cse = new Ter("case","CASE");
		Ter* colon = new Ter("colon","COLON");
		setSon(0,cse);
		setSon(1,num);
		setSon(2,colon);
	}
CaseDec::CaseDec(string a, Node_ptr num_p, int lineno):Node(4){
		default_count=0;
		this->ln=-1;
		Ter* num = static_cast<Ter*>(num_p);
		Ter* cse = new Ter("case","CASE");
		Ter* colon = new Ter("colon","COLON");
		Ter* b = new Ter(a,a);
		setSon(0,cse);
		setSon(1,num);
		setSon(2,b);
		setSon(3,colon);
	}


void FuncDeclPartOne(Node_ptr retType_p, Node_ptr id_p, int lineno){
	Ter* id = static_cast<Ter*>(id_p);
	RetType* retType = static_cast<RetType*>(retType_p);
	if (st.isNameDefined(id->val)){
		output::errorDef(lineno, id->val);
		st.set_prints(false); exit(0);
	}
	Type* t = static_cast<Type*>(retType->getSon(0));
	st.addNameable(Name(id->val,"func",t->str));
	if(id->val == "main"){
		cb.emit("main:");
		cb.emit("move $fp, $sp");
	} else{
		cb.emit(string("_")+id->val+":");
	}

}

void FuncDeclPartTwo(Node_ptr id_p, Node_ptr formals_p, int lineno){
	Ter* id = static_cast<Ter*>(id_p);
	Formals* formals = static_cast<Formals*>(formals_p);
	Name* funcName= st.getNoneConstName(id->val);
	if(funcName==NULL){
		st.set_prints(false); exit(0);
	}
	st.enterFunctionScope(formals->decl.size());
	//TODO: remove the prints
	funcName->update(
			formals->decl);
	st.setFuncParams(formals->decl);
}


void CheckBoolExpression(Node_ptr exp_p, int lineno){
	Exp* exp = static_cast<Exp*>(exp_p);
	if (exp->type!=BOOL_t){
		output::errorMismatch(lineno);
		st.set_prints(false); exit(0);
	}
}

void CheckNoBreak(Node_ptr statement_p, int lineno){
	Statement* statement = static_cast<Statement*>(statement_p);
//	if (statement->is_break){
//		output::errorUnexpectedBreak(statement->ln);
//		st.set_prints(false); exit(0);
//	}
}

void CheckNumExpression(Node_ptr exp_p, int lineno){
	Exp* exp = static_cast<Exp*>(exp_p);
	if (!(exp->type==INT_t || exp->type==BYTE_t)){
		output::errorMismatch(lineno);
		st.set_prints(false); exit(0);
	}
}

int findBadReturnLine(Statements* statements){
	int line =0;
	Statements* curr = statements;
	int i = 0;
	while(true){

		i++;
		if(curr->getNumberOfSons()==1){
			Statement* state = static_cast<Statement*>(statements->getSon(1));
			line = state->ln;
			return line;
		} else {
			try{
				curr = static_cast<Statements*>(curr->getSon(0));
			} catch(...){
				return 0 ;
			}

		}
	}
}

//void checkReturnTypeValidity(Node_ptr retType_p ,Node_ptr statement_p, int lineno){
//	Statements* statements = static_cast<Statements*>(statement_p);
//	RetType* retType = static_cast<RetType*>(retType_p);
//	if(retType->is_void && statements->is_return ){
//		for(list<string>::iterator it = statements->return_types.begin();
//				it!=statements->return_types.end();
//				it++){
//			if(*it != "VOID"){
//				int line = findBadReturnLine(statements);
//				output::errorMismatch(line);
//				st.set_prints(false); exit(0);
//			}
//		}
//
//	}
//	if(!retType->is_void){
//
//		//Type* _type = static_cast<Type*>(retType->getSon(0));
//
//		for(list<string>::iterator it = statements->return_types.begin();
//				it!=statements->return_types.end();
//				it++){
//			if(*it != "NOTVOID"){
//				int line = findBadReturnLine(statements);
//				output::errorMismatch(line);
//				st.set_prints(false); exit(0);
//			}
//		}
//	}
//
//}

