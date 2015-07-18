// synchorinzation objects used for server

#ifndef SERVERSYNCH_H
#define SERVERSYNCH_H

#include "list.h"
#include "network.h"
#include "post.h"
//#include <string>
//#include <sstream>

//using namespace std;

enum lockState {
	SL_FREE, SL_BUSY
};

class serverLock {
public:
	serverLock(char* dName, int owner, int mailbox);
	~serverLock();

	void Acquire(int outAddr, int outBox);
	void Release(int outAddr, int outBox);

	char* name;
	lockState state;
	int ownerID;       //owner machine id
	int mailboxID;      //owner mailbox number
	List* waitQue;

};

class serverCV {
public:
	serverCV(char* dName);
	~serverCV();

	void Signal(serverLock *sLock, int outAddr, int outBox);
	void Wait(serverLock *sLock, int outAddr, int outBox);
	void Boardcast(serverLock *sLock, int outAddr, int outBox);

	char* name;
	serverLock* waitLock;
	List* waitQue;
};

//tableType: 1 for lockTable, 2 for CVTable
bool tableItemExist(char* tName, Table* table, int tableType) {
	bool toReturn = false;

	for (int i = 0; i < table->Size(); i++) {
		if (tableType == 1) {
			serverLock* tableItem = (serverLock*) table->Get(i);
			if (strcmp(tableItem->name, tName) == 0) {
				toReturn = true;
				break;
			}
		} else {
			serverCV* tableItem = (serverCV*) table->Get(i);
			if (strcmp(tableItem->name, tName) == 0) {
				toReturn = true;
				break;
			}
		}

	}

	return toReturn;
}

//get table item's index by name, 1 for lockTable, 2 for CVTable
int getTableIndex(char* tName, Table* table, int tableType) {
	int toReturn = -1;

	for (int i = 0; i < table->Size(); i++) {
		if (tableType == 1) {
			serverLock* tableItem = (serverLock*) table->Get(i);
			if (strcmp(tableItem->name, tName) == 0) {
				toReturn = i;
				break;
			}
		} else {
			serverCV* tableItem = (serverCV*) table->Get(i);
			if (strcmp(tableItem->name, tName) == 0) {
				toReturn = i;
				break;
			}
		}

	}

	return toReturn;
}

void ServerReply(char* sMsg, int outMachine, int outMailbox, int fromMailbox) {
	PacketHeader outPktHdr;
	MailHeader outMailHdr;

	outPktHdr.to = outMachine;
	outMailHdr.to = outMailbox;
	outMailHdr.from = fromMailbox;
	outMailHdr.length = strlen(sMsg) + 1;

	postOffice->Send(outPktHdr, outMailHdr, sMsg);

	delete[] sMsg;
}

#endif
