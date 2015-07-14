#include "serversynch.h"
#include <sstream>
#include <string>

serverLock::serverLock(char* dName, int owner, int mailbox) {
	name = dName;
	ownerID = owner;
	mailboxID = mailbox;
	waitQue = new List();
}

serverLock::~serverLock() {
	delete waitQue;
}

void serverLock::Acquire(int out) {
	PacketHeader outPktHdr, inPktHdr;
	MailHeader outMailHdr, inMailHdr;

	char *msg;
	char buffer[MaxMailSize];
	outPktHdr.to = out;
	outMailHdr.to = 0;
	outMailFrom = 0;
	outMailHdr.length = strlen(msg) +1;

	if (state == FREE) {
		state = BUSY;
		ownerID = out;
		bool success = postOffice->Send(outPktHdr,outMailhdr,msg);

		if (!success) {
			//TODO: handle it
		}

		postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
		fflush (stdout);
	}

	else {
		waitQue->Append(msg);
	}
}

void serverLock::Release(PacketHeader outPkthdr,MailHeader outMailHdr) {

	if (!waitQue->IsEmpty()) {

	}

	else {
		state = FREE;
		ownerID = -1;
	}

}

serverCV::serverCV(char* dName) {
	name = dName;
}

serverCV::~serverCV() {
	delete waitQue;
}

void serverCV::Signal(serverLock *sLock) {

}

void serverCV::Wait(serverLock *sLock) {

}
