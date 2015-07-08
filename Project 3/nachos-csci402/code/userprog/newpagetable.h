// newPageTable.h 
//	TranslationEntry modified for the new page table to hold swap file info

#ifndef NEWPAGETABLE_H
#define NEWPAGETABLE_H

#include "copyright.h"
#include "utility.h"
#include "../machine/translate.h"

class newPageTable : public TranslationEntry {
  public:
    AddrSpace* space;
    int processID;
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
    int swapFilePositionl   // page offset location in the swap file
    bool inSwapFile;    // Whether it is written to the swap file
};

#endif