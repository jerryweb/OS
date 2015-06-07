//This is the passenger class 

#ifndef SECURITYINSPECTOR_H
#define SECURITYINSPECTOR_H

#include "copyright.h"
#include "thread.h"
#include "list.h"
#include "airportobjects.h"

class Securityinspector:public Thread {
	public:
		Securityinspector(int id);
		~Securityinspector();
		void doStuff();

		int GetID();

	private:
		int id;
		List securityLine;
		List returnLine;
}
