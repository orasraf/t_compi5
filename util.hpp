/*
 * util.h
 *
 *  Created on: May 28, 2018
 *      Author: user
 */
#include <string>
#include <iostream>
#include <cstddef>
#include <string>
#include <stdlib.h>


#ifndef T_COMPI3_UTIL_H_
#define T_COMPI3_UTIL_H_

using namespace std;

string getArrType(string type_size);
int getArrSize(string type_size);
string getNameType(string type);
bool isArray(string type);
void resetFile();
void printToFile(string code);


#endif /* T_COMPI3_UTIL_H_ */
