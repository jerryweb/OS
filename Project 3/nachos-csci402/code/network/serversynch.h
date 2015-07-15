// synchorinzation objects used for server

#ifndef SERVERSYNCH_H
#define SERVERSYNCH_H

#include "list.h"
#include "network.h"
#include "post.h"
//TODO:need to include Table here

enum lockState {
	FREE, BUSY
};

//tableType: 1 for lockTable, 2 for CVTable
bool tableItemExist(string tName, Table* table, int tableType) {
	bool toReturn = false;

	for (int i = 0; i < table->Size(); i++) {
		if (tableType == 1)
			serverLock* tableItem = (serverLock*) sTable->Get(i);
		else
			serverCV* tableItem = (serverCV*) sTable->Get(i);
		if (tableItem->name == lName) {
			toReturn = true;
			break;
		}
	}

	return toReturn;
}

bool replyMessage(char* msg,int out) {
}
}

class serverLock {
public:
	serverLock(char* dName, int owner, int mailbox);
	~serverLock();

	void Acquire(int out);
	void Release(int lock);

	string name;
	lockState state;
	int owenerID;       //owner machine id
	int mailboxID;      //owner mailbox number
	List* waitQue;

};

class serverCV {
public:
	serverCV(char* dName);
	~serverCV();

	void Signal(serverLock *sLock,int out);
	void Wait(serverLock *sLock,int out);

	char* name;
	serverLock* lock;
	List* waitQue;
	Table* lockTable;
};

#endif
