/*
 * name.cpp
 *
 *  Created on: Jun 12, 2018
 *      Author: user
 */

#include "name.hpp"

Name::Name(string id, string type):id(id),type(type),returnType(""),numerOfParams(0),parameters(NULL){
		offSet=0;
	}
Name::Name(string id, string type, string ret,int numerOfParams ,pair<string,string>* params):
		id(id),type(type),returnType(ret),numerOfParams(numerOfParams){
		parameters = new pair<string,string>[numerOfParams];
		for(int i=0; i< numerOfParams ; i++){
			parameters[i] = params[i];
		}
		offSet=0;
	}
Name::Name(string id, string type, string ret):
			id(id),type(type),returnType(ret),numerOfParams(0),parameters(NULL){
		offSet=0;
	}
Name::Name(const Name& name){
		offSet=0;
		id = name.id;
		type = name.type;
		numerOfParams = name.numerOfParams;
		returnType = name.returnType;
		if(numerOfParams>0){
			parameters = new pair<string,string>[numerOfParams];
			for(int i=0; i< numerOfParams ; i++){
				parameters[i] = name.parameters[i];
			}
		} else {
			parameters =NULL;
		}
	}
Name::~Name(){
		if(numerOfParams > 0 ){
			delete[] parameters;
		}
	}
	void Name::update(list<pair<string,string> > params){
		numerOfParams = params.size();
		parameters = new pair<string,string>[numerOfParams];
		int i =0;
		for(list<pair<string,string> >::iterator it = params.begin(); it!=params.end(); ++it ){
			parameters[i] = *it;
			i++;
		}
	}
	void Name::setOffSet(int offset){
		offSet = offset;
	}

