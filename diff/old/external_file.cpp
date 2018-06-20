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

#define YYSTYPE Node*

using namespace std;


void closeFunctionScope(){
	st.closeFunctionScope();
}

void openScope(){
	st.enterScope();
}

void closeScope(){
	st.exitScope();
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
	Ter::Ter(string val,string kind):val(val){}
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


Node::Node():numberOfSons(0),sons(NULL){}
Node::Node(int numSons):numberOfSons(numSons){
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
			//--//--////--//--cout << "set number of sons cannot set to less than zero or more than 15" << endl;
			st.set_prints(false); exit(0);
		}
		sons = new Node_ptr[num];
	}
int Node::getNumberOfSons(){
		return numberOfSons;
	}
void Node::setSon(int idx, Node_ptr son_p){
		if(idx<0 || idx >= numberOfSons){
			//--//--////--//--cout << "idx out of bounds at setSons call" << endl;
			st.set_prints(false); exit(0);
		}
		sons[idx] = son_p;
	}
Node_ptr Node::getSon(int idx){
		if(idx<0 || idx >= numberOfSons){
			//--//--////--//--cout << "idx out of bounds at setSons call" <<endl;
			st.set_prints(false); exit(0);
		}
		return sons[idx];
	}


// ======================== IMPLEMENT CALL =====
	Call::Call(Node_ptr id_p, int lineno):Node(3){
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
//				////--//--cout << "expected params: " << myName->numerOfParams << endl;
//				////--//--cout << "received params: " << expTypes.size() << endl;
				if (myName->numerOfParams != expTypes.size()){
					errType=true;
				}
				for(int i=0 ; i< myName->numerOfParams; i++){
					paramList.push_back(myName->parameters[i].first);
//					////--//--cout << "needed param number " << i << " is " << myName->parameters[i].first << endl;
//					////--//--cout << "received param number " << i << " is " << (*expIt) << endl;
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
		}


//============ implementing Exp



	void Exp::EqFunc(types* type, string* val,const Exp& b,const Exp& c,int ln){
		if (b.type==BOOL_t || c.type==BOOL_t || b.type==STRING_t || c.type==STRING_t || b.type==VOID_t || c.type==VOID_t){
			output::errorMismatch(ln);
			st.set_prints(false); exit(0);
		}
		*type=BOOL_t;
		if (b.value.length()>0 && c.value.length()>0){
			if (b.value!=c.value){
				*val="false";
			} else {
				*val="true";
			}
		}
	}

	void Exp::NeqFunc(types* type, string* val,const Exp& b, const Exp& c, int ln){
		if (b.type==BOOL_t || c.type==BOOL_t || b.type==STRING_t || c.type==STRING_t || b.type==VOID_t || c.type==VOID_t){
			output::errorMismatch(ln);
			st.set_prints(false); exit(0);
		}
		*type=BOOL_t;
		if (b.value.length()>0 && c.value.length()>0){
			if (b.value!=c.value){
				*val="true";
			} else {
				*val="false";
			}
		}
	}

	void Exp::SeqFunc(types* type, string* val,const Exp& b,const Exp& c, int ln){
		if (b.type==BOOL_t || c.type==BOOL_t || b.type==STRING_t || c.type==STRING_t || b.type==VOID_t || c.type==VOID_t){
			output::errorMismatch(ln);
			st.set_prints(false); exit(0);
		}
		*type=BOOL_t;
		if (b.value.length()>0 && c.value.length()>0){
			if (atoi(b.value.c_str())<=atoi(c.value.c_str())){
				*val="true";
			} else {
				*val="false";
			}
		}
	}

	void Exp::SmlFunc(types* type, string* val,const Exp& b,const Exp& c, int ln){
		if (b.type==BOOL_t || c.type==BOOL_t || b.type==STRING_t || c.type==STRING_t || b.type==VOID_t || c.type==VOID_t){
			output::errorMismatch(ln);
			st.set_prints(false); exit(0);
		}
		*type=BOOL_t;
		if (b.value.length()>0 && c.value.length()>0){
			if (atoi(b.value.c_str())<atoi(c.value.c_str())){
				*val="true";
			} else {
				*val="false";
			}
		}
	}

	void Exp::MathFunc(types* type, string* val, const Exp& b, const Exp& c, int ln, string op){
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
		if (b.value.length()>0 || c.value.length()>0){
			if (op=="+"){
//TODO:: I put ""'s in Vals because of arithmetic ops should not raise "byte exceeding from 255"
				ss << atoi(b.value.c_str())+atoi(c.value.c_str());
								*val = "";//ss.str();

			} else if (op=="-"){
				ss << atoi(b.value.c_str())-atoi(c.value.c_str());
								*val = "";//ss.str();

			} else if (op=="*"){
				ss << atoi(b.value.c_str())*atoi(c.value.c_str());
								*val ="";//ss.str();

			} else if (op=="/"){
				if (atoi(c.value.c_str())!=0) {
					ss << atoi(b.value.c_str())/atoi(c.value.c_str());
				}
								*val = "";//ss.str();
			}
		}
	}

	void Exp::AndFunc(types* type, string* val, const Exp& b, const Exp& c, int ln){
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
	}

	void Exp::OrFunc(types* type, string* val, const Exp& b, const Exp& c, int ln){
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
	}



	Exp::Exp(string op , Node_ptr leftNode , Node_ptr rightNode, int lineno):
		Node(3),type(UNDEF_t),value(""),lineno(lineno) {

		Exp* left = static_cast<Exp*>(leftNode);
		Exp* right = static_cast<Exp*>(rightNode);

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

		if (op=="=="){
			EqFunc(&myType,&myVal,*left,*right,lineno);
		} else if (op=="!="){
			NeqFunc(&myType,&myVal,*left,*right,lineno);
		} else if (op=="<="){
			SeqFunc(&myType,&myVal,*left,*right,lineno);
		} else if (op==">="){
			SeqFunc(&myType,&myVal,*right,*left,lineno);
		} else if (op=="<"){
			SmlFunc(&myType,&myVal,*left,*right,lineno);
		} else if (op==">"){
			SmlFunc(&myType,&myVal,*right,*left,lineno);
		} else if (op=="+"){
			MathFunc(&myType,&myVal,*left,*right,lineno,"+");
		} else if (op=="-"){
			MathFunc(&myType,&myVal,*left,*right,lineno,"-");
		} else if (op=="*"){
			MathFunc(&myType,&myVal,*left,*right,lineno,"*");
		} else if (op=="/"){
			MathFunc(&myType,&myVal,*left,*right,lineno,"/");
		} else if (op=="AND"){
			AndFunc(&myType,&myVal,*left,*right,lineno);
		} else if (op=="OR"){
			OrFunc(&myType,&myVal,*left,*right,lineno);
		}
		this->type=myType;
		this->value=myVal;

		Op* temp_op = new Op(op);

		this->setSon(0,left);
		this->setSon(1,temp_op);
		this->setSon(2,right);
	}

	Exp::Exp(Node_ptr onlySon, int lineno):Node(2),type(UNDEF_t),value(""),lineno(lineno){
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
		Op* not_op = new Op("NOT");
		this->setSon(0,not_op);
		this->setSon(1,son);
	}

	Exp::Exp(string opa, Node_ptr onlySon, string opb, int lineno):Node(3),type(UNDEF_t),value(""),lineno(lineno){
		Exp* son = static_cast<Exp*>(onlySon);

		this->type=son->type;
		this->value=son->value;

		Ter* lparen = new Ter("(","LPAREN");
		Ter* rparen = new Ter(")","RPAREN");

		this->setSon(0,lparen);
		this->setSon(1,son);
		this->setSon(2,rparen);
	}


	Exp::Exp(string op, Node_ptr onlySon, int lineno):Node(1),type(UNDEF_t),value(""),lineno(lineno){
		if (op=="Call"){
			Call* son = static_cast<Call*>(onlySon);
			this->type=son->type;
		} else {
			Ter* son = static_cast<Ter*>(onlySon);
			if (op=="INT"){
				this->type=INT_t;
			} else if (op=="TRUE" || op=="FALSE"){
				this->type=BOOL_t;
			} else if (op=="B"){
				if(atoi(son->val.c_str())>255){
					output::errorByteTooLarge(lineno,son->val);
					st.set_prints(false); exit(0);
				}
				this->type=BYTE_t;
			} else if (op=="ID"){
				if (!(st.isNameDefined(son->val))){
					output::errorUndef(lineno,son->val);
					st.set_prints(false); exit(0);
				} else {
					Name* temp =st.getNoneConstName(son->val);
					this->type = stringToType(temp->type);
				}
			} else if (op=="STRING"){
				this->type=STRING_t;
			}
			this->value=son->getVal();
		}
		this->setSon(0,onlySon);
	}





//============implemeting ExpList ========
ExpList::ExpList(Node_ptr exp_p, int lineno):Node(1){
		Exp* exp = static_cast<Exp*>(exp_p);
		this->size=1;
		this->type=exp->type;
		string temp = typeToString(exp->type);
		myList.insert(myList.begin(),temp);
		setSon(0,exp);
	}
ExpList::ExpList(Node_ptr exp_p, Node_ptr explist_p, int lineno):Node(3){
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
StatementIF::StatementIF(Node_ptr exp_p, Node_ptr statement_p, int lineno):Node(3){
	//////--//--cout << "in statementIF" << endl;
	CheckNoBreak(statement_p,lineno);
	Exp* exp = static_cast<Exp*>(exp_p);
	Statement* statement = static_cast<Statement*>(statement_p);
	setSon(0,exp);
	setSon(1,statement);
}

//========= implemening Statement
Statement::Statement(string ter, int lineno):Node(2){
		is_break=false;
		is_return=false;
		is_emidiate_return=false;
		if (ter=="BREAK"){
			this->is_break=true;
			this->ln=lineno;
		}
		if (ter=="RETURN"){
			is_emidiate_return=true;
			this->is_return=true;
			pair<types,int> a = make_pair(VOID_t,lineno);
			return_types.insert(return_types.begin(),a);
		}
		Ter* term = new Ter(ter,ter);
		Ter* sc = new Ter(";","SC");
		setSon(0,term);
		setSon(1,sc);
		this->ln=lineno;
	}
Statement::Statement(Node_ptr call_p, int lineno):Node(2){
		is_break=false;
		is_return=false;
		is_emidiate_return=false;
		Call* call = static_cast<Call*>(call_p);
		Ter* sc = new Ter(";","SC");
		setSon(0,call);
		setSon(1,sc);
		this->ln=lineno;
	}
Statement::Statement(string kind, Node_ptr b_p, int lineno):Node(3){
		is_return=false;
		is_break=false;
		is_emidiate_return=false;
		Ter* a;
		Ter* c;
		Exp* b;
		if (kind=="STATEMENTS"){
			a = new Ter("{","LBRACE");
			Statements* b = static_cast<Statements*>(b_p);
			if (b->is_break){
				this->is_break = true;
				this->ln = b->ln;
			}
			if (b->is_return){
				this->is_return=true;
				if (!(b->return_types.size()==0)){
					return_types.insert(return_types.end(), b->return_types.begin(), b->return_types.end());
				}
			}
			c = new Ter("}","RBRACE");
		} else {
			a = new Ter("return","RETURN");
			b = static_cast<Exp*>(b_p);
			c = new Ter(";","SC");
			this->is_return = true;
			pair<types,int> a = make_pair(b->type,lineno);
			this->return_types.insert(return_types.begin(),a);
		}
		setSon(0,a);
		setSon(1,b);
		setSon(2,c);
		this->ln=lineno;
	}
Statement::Statement(Node_ptr type_p, Node_ptr id_p, int lineno):Node(3){
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
		}
		setSon(0,type);
		setSon(1,id);
		setSon(2,sc);
		//this->ln=lineno;
	}
Statement::Statement(string op, Node_ptr id_p, Node_ptr exp_p, int lineno):Node(4){
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
		setSon(0,id);
		setSon(1,ass);
		setSon(2,exp);
		setSon(3,sc);
		this->ln=lineno;
	}

Statement::Statement(Node_ptr if_p, string noneed, int lineno):Node(5){
	StatementIF* statementif = static_cast<StatementIF*>(if_p);
	is_return=false;
	is_break=false;
	is_emidiate_return=false;
	Statement* statement = static_cast<Statement*>(statementif->getSon(1));
	Exp* exp = static_cast<Exp*>(statementif->getSon(0));
	if (statement->is_return){
		//////--//--cout << "get in?" << endl;
		this->is_return=true;
		//////--//--cout << "get out?" << endl;
		if (!(statement->return_types.size()==0)){
			return_types.insert(return_types.end(),statement->return_types.begin(),statement->return_types.end());
		}
	}
	Ter* start = new Ter("IF","IF");
	Ter* lparen = new Ter("(","LPAREN");
	Ter* rparen = new Ter(")","RPAREN");
	setSon(0,start);
	setSon(1,lparen);
	setSon(2,exp);
	setSon(3,rparen);
	setSon(4,statement);
	if (statement->is_break){
		this->is_break=true;
		this->ln=statement->ln;
	}
}

Statement::Statement(Node_ptr exp_p, Node_ptr statement_p, string op, int lineno):Node(5){
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
//			//--//--////--//--cout << " Statement , " << exp->type << endl;
//			output::errorMismatch(lineno);
//			st.set_prints(false); exit(0);
//		}
		Ter* start = new Ter(op,op);
		Ter* lparen = new Ter("(","LPAREN");
		Ter* rparen = new Ter(")","RPAREN");
		setSon(0,start);
		setSon(1,lparen);
		setSon(2,exp);
		setSon(3,rparen);
		setSon(4,statement);
		if (statement->is_break){
			this->is_break=true;
			this->ln=statement->ln;
		}
	}
Statement::Statement(string op1, Node_ptr exp_p, Node_ptr caselist_p, string op2, int lineno):Node(8){
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
					//--//--////--//--cout << "FOUND NULL on Statement: " << id->val << endl;
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
		setSon(0,type);
		setSon(1,id);
		setSon(2,ass);
		setSon(3,exp);
		setSon(4,sc);
		this->ln=lineno;
	}
Statement::Statement(Node_ptr if_p, Node_ptr sb_p, int lineno, int twice):Node(7){
		CheckNoBreak(sb_p,lineno); //orig
		is_return=false;
		is_break=false;
		is_emidiate_return=false;
		StatementIF* statementif = static_cast<StatementIF*>(if_p);
		Statement* sta = static_cast<Statement*>(statementif->getSon(1));
		Statement* stb = static_cast<Statement*>(sb_p);
//		if(sta->is_break || stb->is_break){
//			if(sta->is_break){
//				output::errorUnexpectedBreak(sta->ln);
//				st.set_prints(false); exit(0);
//			}else{
//				output::errorUnexpectedBreak(stb->ln);
//				st.set_prints(false); exit(0);
//			}
//		}
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
		Exp* exp = static_cast<Exp*>(statementif->getSon(1));
//		if (exp->type!=BOOL_t){
//			output::errorMismatch(lineno);
//			st.set_prints(false); exit(0);
//		}
		Ter* start = new Ter("IF","IF");
		Ter* lparen = new Ter("(","LPAREN");
		Ter* rparen = new Ter(")","RPAREN");
		Ter* end = new Ter("ELSE","ELSE");
		setSon(0,start);
		setSon(1,lparen);
		setSon(2,exp);
		setSon(3,rparen);
		setSon(4,sta);
		setSon(5,end);
		setSon(6,stb);
//		this->ln=lineno;
	}


// ============= implementing Statements ========
Statements::Statements(Node_ptr statement_p, int lineno):Node(1){
		is_break=false;
		is_return=false;
		return_types = list<pair<types,int> >() ;
		Statement* statements = static_cast<Statement*>(statement_p);
		if (statements->is_break){
			this->is_break=true;
			this->ln=statements->ln;
		}
		if (statements->is_return){
			this->is_return=true;
			if (!(statements->return_types.size()==0)){
				this->return_types.insert(this->return_types.end(), statements->return_types.begin(), statements->return_types.end());
			}
		}
		setSon(0,statements);
	}
Statements::Statements(Node_ptr statements_p, Node_ptr statement_p, int lineno):Node(2){
		//////--//--cout << "in statements" << endl;
		is_break=false;
		is_return=false;
		return_types = list<pair<types,int> >() ;
		Statements* statements = static_cast<Statements*>(statements_p);
		Statement* statement = static_cast<Statement*>(statement_p);
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
		setSon(0,statements);
		setSon(1,statement);
	}


//================implementing FuncDecl
FuncDecl::FuncDecl(Node_ptr rettype_p, Node_ptr id_p, Node_ptr formals_p, Node_ptr statements_p, int lineno):Node(8){
		//--//--////--//--cout << "000 Entering FuncDecl" << endl;
		is_main = false;
		//////--//--cout << "in func decl" << endl;
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
		//--//--////--//--cout << "]]>>> void main check: " << id->val << " , " << rettype->is_void << " " << formals->is_empty << endl;
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
		setSon(0,rettype);
		setSon(1,id);
		setSon(2,lparen);
		setSon(3,formals);
		setSon(4,rparen);
		setSon(5,lbrace);
		setSon(6,statements);
		setSon(7,rbrace);

	}

// ========== implementing Funcs =====
Funcs::Funcs(Node_ptr funcdecl_p, Node_ptr funcs_p, int lineno):Node(2){
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
		is_main=false;
		//TODO: figure out if this is the way to handle epsilons, and if so, change the parser to match
	}

//==============implement Program
Program::Program(Node_ptr funcs_p, int lineno):Node(1){
		Funcs* funcs = static_cast<Funcs*>(funcs_p);
		if (!(funcs->is_main)){
			output::errorMainMissing();
			st.set_prints(false); exit(0);
		}
		setSon(0,funcs);
	}


//=================== imlemenintg RetType

RetType::RetType(Node_ptr type_p, int lineno):Node(1){
		//////--//--cout << "in rettype" << endl;
		is_void = false;
		Type* type = static_cast<Type*>(type_p);
		this->type = type->val;
		setSon(0,type);
	}
RetType::RetType(int lineno):Node(1){
		//////--//--cout << "in void rettype" << endl;
		Ter* v = new Ter("VOID","VOID");
		this->is_void = true;
		this->type=VOID_t;
		setSon(0,v);
	}

//====== implementing Formals
Formals::Formals(Node_ptr formalslist_p, int lineno):Node(1){
		//////--//--cout << "in formals" << endl;
		is_empty = false;
		FormalsList* formalslist = static_cast<FormalsList*>(formalslist_p);
		this->decl = formalslist->decl;
		setSon(0,formalslist);
	}
Formals::Formals(int lineno):Node(){
		//////--//--cout << "in empty formals" << endl;
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
		} else {
			this->ln=-1;
		}
		////--//--cout << "single caselist is: " << ln << endl;
		setSon(0,casestatement);
	}
CaseList::CaseList(Node_ptr caselist_p, Node_ptr casestatement_p, int lineno):Node(2){
		default_count=0;
		CaseList* caselist = static_cast<CaseList*>(caselist_p);
		CaseStatement* casestatement = static_cast<CaseStatement*>(casestatement_p);
		this->default_count = casestatement->default_count + caselist->default_count;
		if (this->default_count>1){
			////--//--cout << "casestatement is: " << casestatement->ln1 << endl;
			////--//--cout << "caselist is: " << caselist->ln << endl;
			int temp = maxilin(casestatement->ln1,caselist->ln);
			output::errorTooManyDefaults(temp);
			st.set_prints(false); exit(0);
		} else {
			if (casestatement->ln1>0){
				this->ln = casestatement->ln1;
			} else if (caselist->ln>0){
				this->ln = caselist->ln;
			}
		}
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
		}
		setSon(0,casedec);
	}
CaseStatement::CaseStatement(Node_ptr casedec_p, Node_ptr statements_p, int lineno):Node(2){
		default_count=0;
		this->ln1=-1;
		CaseDec* casedec = static_cast<CaseDec*>(casedec_p);
		Statements* statements = static_cast<Statements*>(statements_p);
		if(casedec->default_count!=0){
			this->default_count++;
			this->ln1=casedec->ln;
		}
		setSon(0,casedec);
		setSon(1,statements);
	}

//===========IMPLEMENTING CaseDec
CaseDec::CaseDec(int lineno):Node(2){
		default_count=0;
		Ter* deft = new Ter("default","DEFAULT");
		Ter* colon = new Ter("colon", "COLON");
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
	//////--//--cout << "in func decl part 1" << endl;
}

void FuncDeclPartTwo(Node_ptr id_p, Node_ptr formals_p, int lineno){
	Ter* id = static_cast<Ter*>(id_p);
	Formals* formals = static_cast<Formals*>(formals_p);
	Name* funcName= st.getNoneConstName(id->val);
	if(funcName==NULL){
		//--//--////--//--cout <<"FFFFFFUUUCK"<<endl;
		st.set_prints(false); exit(0);
	}
	st.enterFunctionScope(formals->decl.size());
	//TODO: remove the prints
	////--//--////--//--cout << formals->decl.size() << endl;
	funcName->update(
			formals->decl);
	st.setFuncParams(formals->decl);
	//////--//--cout << "in func decl part 2" << endl;
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

		////--//--cout<< "loop #" << i <<endl;
		i++;
		if(curr->getNumberOfSons()==1){
			Statement* state = static_cast<Statement*>(statements->getSon(1));
			line = state->ln;
			////--//--cout << " line is " <<  line << endl;
			return line;
		} else {
			try{
				curr = static_cast<Statements*>(curr->getSon(0));
			} catch(...){
				////--//--cout << "failed getSon(0)" << endl;
				return 0 ;
			}

		}
	}
}

//void checkReturnTypeValidity(Node_ptr retType_p ,Node_ptr statement_p, int lineno){
//	////--//--cout << "ret 1 " << endl;
//	Statements* statements = static_cast<Statements*>(statement_p);
//	RetType* retType = static_cast<RetType*>(retType_p);
//	////--//--cout <<"rettype: " << retType->getNumberOfSons() << endl;
//	if(retType->is_void && statements->is_return ){
//		for(list<string>::iterator it = statements->return_types.begin();
//				it!=statements->return_types.end();
//				it++){
//			////--//--cout << "..." << *it << endl;
//			if(*it != "VOID"){
//				int line = findBadReturnLine(statements);
//				output::errorMismatch(line);
//				st.set_prints(false); exit(0);
//			}
//		}
//
//	}
//	////--//--cout << "ret 2 " << endl;
//	if(!retType->is_void){
//		////--//--cout << "ret 3 " << endl;
//
//		//Type* _type = static_cast<Type*>(retType->getSon(0));
//
//		for(list<string>::iterator it = statements->return_types.begin();
//				it!=statements->return_types.end();
//				it++){
//			////--//--cout << *it << " =?= " << *it << endl;
//			if(*it != "NOTVOID"){
//				int line = findBadReturnLine(statements);
//				output::errorMismatch(line);
//				st.set_prints(false); exit(0);
//			}
//		}
//	}
////	//--//--cout << "ret 4 " << endl;
//
//}

