// translateexec.h 
//	TranslationEntry modified for virtual memory stuff, I think?

#ifndef TRANSLATEEXEC_H
#define TRANSLATEEXEC_H

#include "copyright.h"
#include "utility.h"
#include "../machine/translate.h"

enum PageLoc {NEITHER, EXEC, SWAP};

class TranslationEntryExec : public TranslationEntry {
  public:
    PageLoc location;     // Location of the page (enum int)
    int byteOffset;     // Page location in executable file (if EXEC) or swap file (if SWAP).
    int virtualPage;  	// The page number in virtual memory.
    int physicalPage;  	// The page number in real memory (relative to the
			//  start of "mainMemory"
    bool valid;         // If this bit is set, the translation is ignored.
			// (In other words, the entry hasn't been initialized.)
    bool readOnly;	// If this bit is set, the user program is not allowed
			// to modify the contents of the page.
    bool use;           // This bit is set by the hardware every time the
			// page is referenced or modified.
    bool dirty;         // This bit is set by the hardware every time the
			// page is modified.
    //int swapFilePositionl   // page offset location in the swap file
};

#endif
