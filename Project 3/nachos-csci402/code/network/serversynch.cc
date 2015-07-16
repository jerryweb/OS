#include "serversynch.h"
#include "interrupt.h"
#include <sstream>

serverLock::serverLock(char* dName, int owner, int mailbox) {
	name = dName;
	ownerID = owner;
	mailboxID = mailbox;
	waitQue = new List();
}

serverLock::~serverLock() {
	delete waitQue;
}

void serverLock::Acquire(int out,int mailbox) {
	PacketHeader outPktHdr, inPktHdr;
	MailHeader outMailHdr, inMailHdr;
	char *msg;
	char buffer[MaxMailSize];
	stringstream ss;
	ss.str("");
	ss.clear;

	outPktHdr.to = out;
	outMailHdr.to = 0;
	outMailHdr.from = out;

	if (state == FREE) {
		msg = "0";
		outMailHdr.length = strlen(msg) +1;

		//proceed after acknowledgement
		state = BUSY;
		ownerID = out;

	}

	else {
		//append the machine index, since the message is always the same
		int* toAppend = new int[1];
		toAppend[0] = out;
		waitQue->Append(toAppend);
	}
}

void serverLock::Release() {

	if (!waitQue->IsEmpty()) {
		int* outPtr = (int*)waitQue->First();
		int out = outPtr[0];

		PacketHeader outPktHdr, inPktHdr;
		MailHeader outMailHdr, inMailHdr;
		char *msg;
		char buffer[MaxMailSize];
		stringstream ss;
		ss.str("");
		ss.clear;

		outPktHdr.to = out;
		outMailHdr.to = 0;
		outMailHdr.from = out;

		msg = "0";
		outMailHdr.length = strlen(msg) +1;

		waitQue->Remove();
		delete [] outPtr;
	}

	else {
		state = FREE;
		ownerID = -1;
	}

}

serverCV::serverCV(char* dName,Table* lTable) {
	name = dName;
	lock = NULL;
	waitQue = new List();
	lockTable = lTable;
}

serverCV::~serverCV() {
	delete waitQue;
}

void serverCV::Signal(serverLock *sLock,int out) {
	PacketHeader outPktHdr, inPktHdr;
	MailHeader outMailHdr, inMailHdr;
	char *msg;
	char buffer[MaxMailSize];
	stringstream ss;
	ss.str("");
	ss.clear;

	outPktHdr.to = out;
	outMailHdr.to = 0;
	outMailHdr.from = out;

	//check if lock exist
	if (!tableItemExist(sLock->name,lockTable,1)) {
		msg = "1";
	} else {

	}
}

void serverCV::Wait(serverLock *sLock,int out) {
	PacketHeader outPktHdr, inPktHdr;
	MailHeader outMailHdr, inMailHdr;
	char *msg;
	char buffer[MaxMailSize];
	stringstream ss;
	ss.str("");
	ss.clear;

	outPktHdr.to = out;
	outMailHdr.to = 0;
	outMailHdr.from = out;


}
