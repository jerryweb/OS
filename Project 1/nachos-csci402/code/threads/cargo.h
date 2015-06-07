//This is the check-in staff class 

#ifndef CARGO_H
#define CARGO_H

#include "copyright.h"
#include "thread.h"
#include "list.h"
#include "airportobjects.h"

class Cargo : public Thread {
	public:
		Cargo(int id, Airport* airport);
		~Cargo();
		void doStuff();

	private:
		int id;
        Airport* airport;
        int luggage[3];
        int weight[3];
}