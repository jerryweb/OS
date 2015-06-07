//This is the liaison class 

#ifndef LIAISON_H
#define LIAISON_H

#include "copyright.h"
#include "thread.h"
#include "list.h"
#include "airportobjects.h"

class Liaison : public Thread {
	public:
		Liaison(int id, Airport* airport);
		~Liaison();
		void doStuff();

	private:
		int id;
        Airport* airport;
        int passengers[3]; // assumes 3 airlines
        int luggage[3];
}