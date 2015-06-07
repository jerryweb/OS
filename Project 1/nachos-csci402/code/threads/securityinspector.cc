#include "securityinspector.h"

Securityinspector::Securityinspector(int id)
{
	this.id = id;
}

Securityinspector::~Securityinspector(){

}

void Securityinspector::doStuff(int* liaisonList[7])
{
	while(true) {
		//starting C.S.

		//handle return line first
		if (returnLine.Size() != 0) {

		} else {

		}
	}
}

int Securityinspector::GetID() {
	return this.id;
}
