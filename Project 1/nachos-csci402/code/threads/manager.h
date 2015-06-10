// Manager class.

#ifndef MANAGER_H
#define MANAGER_H

#include "copyright.h"
#include "thread.h"
#include "list.h"
#include "airport.h"

class Manager {
	public:
		Manager(Airport* airport);
		~Manager();
		void Run();

	private:
        Airport* airport;   // "Airport" construct, containing all public data.
        // TODO: add more variables
};

#endif
