/*
 * scopes_DS.hpp
 *
 *  Created on: Jun 12, 2018
 *      Author: user
 */

#ifndef T_COMPI5_SCOPES_DS_HPP_
#define T_COMPI5_SCOPES_DS_HPP_

#include "name.hpp"
#include <iostream>
#include <cstddef>
#include <string>
#include <stdlib.h>
#include <map>
#include <vector>
#include <list>
#include <stack>
#include "output.hpp"
#include "util.hpp"

class SymbolsTable{
	int _while;
	int _switch;
	map<string, Name*> symbols_map;
	vector<int> offsets_stack;
	vector<pair<Name*,int> > name_scope_tuple_stack;
	vector<Name*> functions_vector;
	int scopeDepth ;
	bool prints;
public:
	SymbolsTable();
	~SymbolsTable();
	int getWhileCount();
	void incWhile();
	void decWhile();
	int getSwitchCount();
	void incSwitch();
	void decSwitch();
	void set_prints(bool b);
	void exitScope();
	void enterScope();
	void enterFunctionScope(int numberOfParams);
	void closeFunctionScope();
	bool isNameDefined(string id);


	void addNameable(Name name);
	const Name* getName(string id);
	Name* getNoneConstName(string id);
	void setFuncParams(list<pair<string,string> > params, int lineno);

	void printOffSet();
	void printOffSet(string id);
	void printFunc();
//	void printCurrScope(){
//		int scopenum = this->offsets_stack.size();
//		  for (std::map<char,int>::iterator it=symbols_map.begin(); it!=symbols_map.end(); ++it){
//			  if(((*it).second))
//		  }
//		    std:://--//--cout << it->first << " => " << it->second << '\n';
//	}
};


#endif /* T_COMPI5_SCOPES_DS_HPP_ */
