/*
 * util.cpp
 *
 *  Created on: May 28, 2018
 *      Author: user
 */
#include <iostream>
#include <cstddef>
#include <string>
#include <stdlib.h>
#include "output.hpp"


string getArrType(string type_size){
	string type = "";
	for(string::iterator ite = type_size.begin() ; ite!= type_size.end() ; ite++){
		if(*ite != '_'){
			type += *ite;
		} else {
			break;
		}
	}
	return type;
}

int getArrSize(string type_size){
	string size_str = "";
	bool start = false;
	for(string::iterator ite = type_size.begin() ; ite!= type_size.end() ; ite++){
		if(start){
			size_str += *ite;
		}
		if(*ite == '_'){
			start = true;
		}
	}
	return atoi(size_str.c_str());
}

string getNameType(string type){
	bool array_type = false;
	for(string::iterator ite = type.begin() ; ite!= type.end() ; ite++){
			if(*ite == '_'){
				array_type = true;
				break;
			}
	}
	if(array_type){
		const string arr_type = getArrType(type);
		int arr_size = getArrSize(type);
		return output::makeArrayType(arr_type,arr_size);
	}
	return type;
}

bool isArray(string type) {
	for (string::iterator ite = type.begin(); ite != type.end(); ite++) {
		if (*ite == '_') {
			return true;
		}
	}
	return false;
}


