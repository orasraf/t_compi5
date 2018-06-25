/*
 * name.hpp
 *
 *  Created on: Jun 12, 2018
 *      Author: user
 */

#ifndef T_COMPI5_NAME_HPP_
#define T_COMPI5_NAME_HPP_

#include <iostream>
#include <cstddef>
#include <string>
#include <stdlib.h>
#include <map>
#include <vector>
#include <list>
#include <stack>
#include "output.hpp"

class Name{
public:
	Name(string id, string type);
	Name(string id, string type, string ret,int numerOfParams ,pair<string,string>* params);
	Name(string id, string type, string ret);
	Name(const Name& name);
	~Name();
	void update(list<pair<string,string> > params);
	void setOffSet(int offset);

	string id; // the name. like int BLABLA . so BLABLA is the name / id
	//string hash; // basically how to get to the name from the root. EX: (2_5_11_3)
	string type; // int , bool , func, etc ..
	string returnType;
	int numerOfParams;
	pair<string,string>* parameters;
	int offSet ;

};



#endif /* T_COMPI5_NAME_HPP_ */
