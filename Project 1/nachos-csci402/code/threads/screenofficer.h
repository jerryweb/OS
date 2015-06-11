#ifndef SCREENOFFICER_H
#define SCREENOFFICER_H

#include "copyright.h"
#include "thread.h"
#include "list.h"
#include "airportobjects.h"
#include "passenger.h"

class ScreenOfficer {
public:
	ScreenOfficer(int ID, Airport* AIRPORT);
	~ScreenOfficer();
	void Screen();

private:
	int id;
	Airport* airport;
};

#endif
