//This is the passenger class 

#ifndef SCREENOFFICER_H
#define SCREENOFFICER_H

#include "copyright.h"
#include "thread.h"
#include "list.h"
#include "airportobjects.h"

class Screenofficer:public Thread {
	public:
		Screenofficer(int id);
		~Screenofficer();
		void Screen();

	private:
		int id;
		List screenLine;

}
