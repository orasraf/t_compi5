/*
 * external_file.hpp
 *
 *  Created on: Dec 18, 2017
 *      Author: user
 */

#ifndef EXTERNAL_FILE_HPP_
#define EXTERNAL_FILE_HPP_

#include <list>
#include <vector>
#include <iostream>
#include <cstddef>
#include <string>
#include "scopes_DS.cpp"

using namespace std;

extern SymbolsTable st;
extern int program_main;

class Node;
typedef Node* Node_ptr;

void openScope();
void closeScope();


class Node{
	int numberOfSons;
	Node_ptr* sons;
public:
	Node();
	Node(int numSons);
	~Node();
	void setNumberOfSons(int num);
	int getNumberOfSons();
	void setSon(int idx, Node_ptr son_p);
	Node_ptr getSon(int idx);
	string name;
};

class Op : public Node{
	string val;
public:
	Op(string val):val(val){}
};

class Ter: public Node{
public:
	string kind;
	string val;

	Ter(string val,string kind);
	string getVal();
};

enum types  {
		STRING_t,
		UNDEF_t,
		BOOL_t,
		INT_t,
		BYTE_t,
		VOID_t
};


class RetType : public Node{
public:
	bool is_void;
	types type;


	RetType(Node_ptr type_p, int lineno);
	RetType(int lineno);
};


class Program : public Node{

public:
	Program(Node_ptr funcs_p, int lineno);
};
class Funcs : public Node{
public:
	bool is_main;
	string f_list;

	Funcs(Node_ptr funcdecl_p, Node_ptr funcs_p, int lineno);
	Funcs(int lineno);
};

class FuncDecl : public Node{
public:
	bool is_main;
	string f_name;
	FuncDecl(Node_ptr rettype_p, Node_ptr id_p, Node_ptr formals_p, Node_ptr statements_p, int lineno);
};

class Formals : public Node{
public:
	bool is_empty;
	list<pair<string,string> > decl;


	Formals(Node_ptr formalslist_p, int lineno);
	Formals(int lineno);
};
class FormalsList : public Node{
public:
	int count_formals ;
	list<pair<string,string> > decl;


	FormalsList(Node_ptr formaldecl_p, int lineno);
	FormalsList(Node_ptr formaldecl_p, Node_ptr formalslist_p, int lineno);
};
class FormalDecl : public Node{
public:
	pair<string,string> nameable;
	FormalDecl(Node_ptr type_p, Node_ptr id_p, int lineno);
	//==================== ARRAY ======================
	FormalDecl(Node_ptr type_p, Node_ptr id_p , Node_ptr arr_size_p , string rule , int lineno);
};
class Statements: public Node{
public:
	bool is_break ;
	bool is_return;
	list<pair<types,int> > return_types;
	int ln;


	Statements(Node_ptr statement_p, int lineno);
	Statements(Node_ptr statements_p, Node_ptr statement_p, int lineno);
};

class StatementIF : public Node{
public:
	StatementIF(Node_ptr exp_p, Node_ptr statement_p, int lineno);
};

class Statement : public Node{
public:
	bool is_break ;
	bool is_return;
	bool is_emidiate_return ;
	list<pair<types,int> > return_types;
	int ln ; //currently this only indicates the line of the break, if found.


	Statement(string ter, int lineno);
	Statement(Node_ptr call_p, int lineno);
	Statement(string kind, Node_ptr b_p, int lineno);
	Statement(Node_ptr type_p, Node_ptr id_p, int lineno);
	Statement(string op, Node_ptr id_p, Node_ptr exp_p, int lineno);
	Statement(Node_ptr if_p, string noneed, int lineno);
	Statement(Node_ptr exp_p, Node_ptr statement_p, string op, int lineno); //WHILE
	Statement(string op1, Node_ptr exp_p, Node_ptr caselist_p, string op2, int lineno);
	Statement(Node_ptr type_p, Node_ptr id_p, Node_ptr exp_p, int lineno);
	Statement(Node_ptr if_p, Node_ptr sb_p, int lineno, int twice);
	//============================ ARRAY ==============================================
	Statement(Node_ptr type_p , Node_ptr id_p , Node_ptr arr_size_p , string rule , int lineno);
	Statement(Node_ptr id_p , Node_ptr exp1_p , Node_ptr exp2_p , string rule , string dummy , int lineno);
};

int maxilin(int ln1, int ln2);

class CaseList : public Node{
public:
	int default_count;
	int ln;

	CaseList(Node_ptr casestatement_p, int lineno);
	CaseList(Node_ptr caselist_p, Node_ptr casestatement_p, int lineno);
};
class CaseStatement : public Node{
public:
	int default_count;
	int ln1;

	CaseStatement(Node_ptr casedec_p, int lineno);
	CaseStatement(Node_ptr casedec_p, Node_ptr statements_p, int lineno);
};
class CaseDec : public Node{
public:
	int default_count;
	int ln;

	CaseDec(int lineno);
	CaseDec(Node_ptr num_p, int lineno);
	CaseDec(string a, Node_ptr num_p, int lineno);
};
class Call : public Node{
public:
	types type;
	Call(Node_ptr id_p, int lineno);
	Call(Node_ptr id_p, Node_ptr expList_p, int lineno);
};

class ExpList : public Node{

//	int size=0;
//	types type=UNDEF;
//	list<string> myList;

public:
	int size;
	types type;
	vector<string> myList;

	ExpList(Node_ptr exp_p, int lineno);
	ExpList(Node_ptr exp_p, Node_ptr explist_p, int lineno);
};
class Type : public Node{

public:
	types val;
	string str;

	Type(string a, int lineno);
};

class Exp : public Node{
public:
	types type;
	string arr_type;
	string value;
	bool is_array;
	int lineno;
private:
// ========================
	void EqFunc(types* type, string* val,const Exp& b,const Exp& c,int ln);

	void NeqFunc(types* type, string* val,const Exp& b, const Exp& c, int ln);

	void SeqFunc(types* type, string* val,const Exp& b,const Exp& c, int ln);

	void SmlFunc(types* type, string* val,const Exp& b,const Exp& c, int ln);

	void MathFunc(types* type, string* val, const Exp& b, const Exp& c, int ln, string op);

	void AndFunc(types* type, string* val, const Exp& b, const Exp& c, int ln);

	void OrFunc(types* type, string* val, const Exp& b, const Exp& c, int ln);


public:
	Exp(string op , Node_ptr leftNode , Node_ptr rightNode, int lineno);
	Exp(Node_ptr onlySon, int lineno);
	Exp(string opa, Node_ptr onlySon, string opb, int lineno);

	Exp(string op, Node_ptr onlySon, int lineno);
	//======================= ARRAY =======================
	Exp(Node_ptr id_p, Node_ptr arr_idx_p, string rule , int lineno);
};

void FuncDeclPartOne(Node_ptr retType_p, Node_ptr id_p, int lineno);
void FuncDeclPartTwo(Node_ptr id_p, Node_ptr formals_p, int lineno);
void CheckBoolExpression(Node_ptr exp_p, int lineno);
void CheckNoBreak(Node_ptr statement_p, int lineno);
void CheckNumExpression(Node_ptr exp_p, int lineno);
void checkReturnTypeValidity(Node_ptr retType_p , Node_ptr statement_p, int lineno);

void closeFunctionScope();

#define YYSTYPE Node*


#endif /* EXTERNAL_FILE_HPP_ */
