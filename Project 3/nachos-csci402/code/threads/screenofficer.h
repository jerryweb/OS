#ifndef SCREENOFFICER_H
#define SCREENOFFICER_H

#include "copyright.h"
#include "thread.h"
#include "list.h"
#include "airportobjects.h"
#include "passenger.h"
#include "securityinspector.h"
#include "stdlib.h"
#include "time.h"

class ScreenOfficer {
public:
	ScreenOfficer(int ID, Airport* AIRPORT);
	~ScreenOfficer();
	void Screen();
	int getID() {return id;}
	Passenger* CheckForPassengers();

private:
	int id;
	Airport* airport;
};

#endif
