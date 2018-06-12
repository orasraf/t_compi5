#include <iostream>
#include <cstddef>
#include <string>
#include <stdlib.h>
#include <map>
#include <vector>
#include <list>
#include <stack>
#include "output.hpp"


using namespace std;


class Name{
public:
	Name(string id, string type):id(id),type(type),returnType(""),numerOfParams(0),parameters(NULL){
		offSet=0;
	}
	Name(string id, string type, string ret,int numerOfParams ,pair<string,string>* params):
		id(id),type(type),returnType(ret),numerOfParams(numerOfParams){
		parameters = new pair<string,string>[numerOfParams];
		for(int i=0; i< numerOfParams ; i++){
			parameters[i] = params[i];
		}
		offSet=0;
	}
	Name(string id, string type, string ret):
			id(id),type(type),returnType(ret),numerOfParams(0),parameters(NULL){
		offSet=0;
	}
	Name(const Name& name){
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
	~Name(){
		if(numerOfParams > 0 ){
			delete[] parameters;
		}
	}
	void update(list<pair<string,string> > params){
		numerOfParams = params.size();
		parameters = new pair<string,string>[numerOfParams];
		int i =0;
		for(list<pair<string,string> >::iterator it = params.begin(); it!=params.end(); ++it ){
			parameters[i] = *it;
			i++;
		}
	}
	void setOffSet(int offset){
		offSet = offset;
	}

	string id; // the name. like int BLABLA . so BLABLA is the name / id
	//string hash; // basically how to get to the name from the root. EX: (2_5_11_3)
	string type; // int , bool , func, etc ..
	string returnType;
	int numerOfParams;
	pair<string,string>* parameters;
	int offSet ;

};

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
	SymbolsTable(){
		_while= 0;
		_switch=0;
		prints=true;
		offsets_stack = vector<int>();
		scopeDepth= 0;
		enterScope();
		//offsets_stack.push_back(0);
	}
	~SymbolsTable(){

		//cout << " --- Destructor --- " << endl;
		for(vector<pair<Name*,int> >::iterator it = name_scope_tuple_stack.begin() ; it != name_scope_tuple_stack.end()
		; it++){
			//--//--cout << (it->first)->id << endl;
		}
		while(!name_scope_tuple_stack.empty()){
			exitScope();
		}
		exitScope(); // for the one in the constructor
		while(!functions_vector.empty()){
			Name* func = *(--functions_vector.end());
			functions_vector.pop_back();
			delete func;
		}


	}
	int getWhileCount(){
		return _while;
	}
	void incWhile(){
		_while++;
	}
	void decWhile(){
		_while--;
	}
	int getSwitchCount(){
		return _switch;
	}
	void incSwitch(){
		_switch++;
	}
	void decSwitch(){
		_switch--;
	}
	void set_prints(bool b){
		prints=b;
	}
	void exitScope(){
		//--cout << "... scope depth (Before Exit) = " << scopeDepth << endl;
		if(scopeDepth == 0){

			//--//--cout << "there are no scopes to exit" << endl;
			set_prints(false) ; exit(0);
		}


		vector<pair<Name*,int> >::iterator it = --name_scope_tuple_stack.end();
//		//--//--cout << it->second << endl;
//		//--//--cout << (int)offsets_stack.size() << endl;
//		char a ;
//		a = getchar() ;
		if(prints){
			output::endScope();
		}


		for(vector<pair<Name*,int> >::reverse_iterator it = name_scope_tuple_stack.rbegin() ;
				it!=name_scope_tuple_stack.rend();
				it++){
			//--cout << " ::: ExitSCope check ::: "<<(*it).second << " =?= " <<  scopeDepth << endl;
			if((*it).second == scopeDepth){
				Name* name = (*it).first;
				if(name->offSet>=0){
					continue;
				}
				string type ;
					type = name->type;

				if(prints){
					output::printID(name->id,name->offSet,type);

				}
			}
		}
		for(vector<pair<Name*,int> >::iterator it = name_scope_tuple_stack.begin() ;
					it!=name_scope_tuple_stack.end();
					it++){
				//--cout << " ::: ExitSCope check ::: "<<(*it).second << " =?= " <<  scopeDepth << endl;
				if((*it).second == scopeDepth){
					Name* name = (*it).first;
					if(name->offSet<0){
						continue;
					}
					string type ;

						type = name->type;

					if(prints){
						output::printID(name->id,name->offSet,type);

					}
				}
			}

		if(scopeDepth == 1 ){
			//printFunc();
			for(vector<Name*>::iterator it = functions_vector.begin(); it != functions_vector.end() ; it++){
				Name* name = (*it);
				pair<string,string>* params = name->parameters;
				vector<string> params_vec;
				//cout << "number of params : " << name->numerOfParams << endl;
				for(int i =0; i < name->numerOfParams ; i++){
					params_vec.push_back(params[i].first);
				}
				string type = output::makeFunctionType(name->returnType,params_vec);
				//cout<< "my test : " << params_vec.size() << endl;
				if(prints){
					output::printID(name->id,0,type);
				}
			}
		}
		functions_vector.size();
		while(!name_scope_tuple_stack.empty()
				&& it->second == scopeDepth){
			Name* name =  it->first;
			symbols_map.erase(name->id);
			delete it->first;
			name_scope_tuple_stack.pop_back();
			if(name_scope_tuple_stack.empty()){
				break;
			}
			it = --name_scope_tuple_stack.end();
		}
		offsets_stack.pop_back();
		scopeDepth--;
	}
	void enterScope(){
		scopeDepth++;
		//--cout << "...scopeDepth (After Enter) = " << scopeDepth << endl;
		if(offsets_stack.empty()){
			offsets_stack.push_back(0);
			return;
		}
		offsets_stack.push_back(  (*(--offsets_stack.end()))  );
	}
	void enterFunctionScope(int numberOfParams){
		scopeDepth++;
		offsets_stack.push_back(-1*numberOfParams);
	}
	void closeFunctionScope(){
		exitScope();
	}
	bool isNameDefined(string id){
		//cout << "isNameDefined : " << id ;
		for(vector<pair<Name*,int> >::iterator it = name_scope_tuple_stack.begin();
				it != name_scope_tuple_stack.end(); ++it){
			if (it->first->id == id){
				//cout << " true" << endl;
				return true;
			}
		}
		for(vector<Name*>::iterator it = functions_vector.begin();
				it != functions_vector.end(); ++it){
			if((*it)->id == id){
				//cout << " true" << endl;
				return true;
			}
		}
		//cout << " false" << endl;
		return false;
	}
	//openFuncionScope

	//closeFunctionScope

//	void addFunction(Name name){
//		//add the function to the current scope without offset - to functions_vector
//		addNameable(name);
//		//open a new scope
//		enterFunctionScope(name.numerOfParams);
//		//add the parameters as -3 -2 -1 ... 0 is the function to allow recursion
//		for(int i=name.numerOfParams-1 ; i>=0 ; i--){
//			string param_name = name.parameters[i].first;
//			string param_type = name.parameters[i].second;
//			addNameable(Name(param_name,param_type));
//		}
//		addNameable(Name(name.id,"func_scope")); // changed the type name a little  to avoid endless recursion
//
//	}

	void addNameable(Name name){
		//--//--cout << "addNameable : " << name.id  << ", "<< name.type << endl;
		if(isNameDefined(name.id)){
			//--//--cout << name.id << " is already defined. no shadowing allowed." << endl;
			cout << " already definied : " << name.id << endl;
			set_prints(false) ; exit(0);
		}

		Name* name_p = new Name(name);


		if(name.type != "func"){
			name_scope_tuple_stack.push_back(pair<Name*,int>(name_p,scopeDepth));
			name_p->setOffSet(*(--offsets_stack.end()));
			(*(--offsets_stack.end()))++;
		} else {
			functions_vector.push_back(name_p);
		}


		symbols_map.insert(pair<string,Name*>(name.id,name_p));

	}
	const Name* getName(string id){
		return symbols_map[id];
	}
	Name* getNoneConstName(string id){
		return symbols_map[id];
	}
	void setFuncParams(list<pair<string,string> > params){
		offsets_stack.push_back(-1*params.size());
		for(list<pair<string,string> >::reverse_iterator it = params.rbegin(); it!=params.rend(); ++it ){
			addNameable(Name(it->second,it->first));
		}
	}

	void printOffSet(){
		if(name_scope_tuple_stack.empty()){
			//--//--cout << "empty" << endl;
			return;
		}
		pair<Name*,int> top = *(--name_scope_tuple_stack.end());
		//--//--cout << top.first->id << " : " << top.second << " : " << *(--offsets_stack.end()) << endl ;
	}
	void printOffSet(string id){
		Name* name = symbols_map[id];
		//--//--cout << name->id << " : offset = " << name->offSet << endl;
	}
	void printFunc(){
		if(functions_vector.empty()){
			//cout<< "No functions defined." << endl;
		}
		//cout << "number of functions is : " << functions_vector.size() << endl;
		for(vector<Name*>::iterator it = functions_vector.begin() ; it != functions_vector.end() ; it++){
			Name* func = *it;
			//cout << ">>>name: " <<  func->id << " , " << "retType: " << func->returnType << ", " << "params: "  ;
			for(int i=0 ; i<func->numerOfParams ; i++){
				cout << func->parameters[i].first << " " << func->parameters[i].second << " ,  ";
			}
			cout << endl;
		}

	}
//	void printCurrScope(){
//		int scopenum = this->offsets_stack.size();
//		  for (std::map<char,int>::iterator it=symbols_map.begin(); it!=symbols_map.end(); ++it){
//			  if(((*it).second))
//		  }
//		    std:://--//--cout << it->first << " => " << it->second << '\n';
//	}
};

//class OffsetStack{
//	list<int> offsetList;
//public:
//	void addOffSet()
//};

