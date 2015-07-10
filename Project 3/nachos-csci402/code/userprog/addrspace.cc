// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "../threads/system.h"
#include "addrspace.h"
#include "noff.h"
// #include "table.h"
// #include "../threads/synch.h"
#include <stdio.h>      /* printf, NULL */
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */


// forward declarations
int getFreePage();
int getPPN(int vpn);

extern "C" { int bzero(char *, int); };
TranslationEntryIPT* ipt;
Lock* iptLock;
BitMap* swapFileMap;
int currentTLB;
int evictionPolicy;
OpenFile* swapFile;
OpenFile* debugFile;

Table::Table(int s) : map(s), table(0), lock(0), size(s) {
    table = new void *[size];
    lock = new Lock("TableLock");
    count = 0;
}

Table::~Table() {
    if (table) {
	delete table;
	table = 0;
    }
    if (lock) {
	delete lock;
	lock = 0;
    }
}

void *Table::Get(int i) {
    // Return the element associated with the given i, or 0 if
    // there is none.

    return (i >=0 && i < size && map.Test(i)) ? table[i] : 0;
}

int Table::Put(void *f) {
    // Put the element in the table and return the slot it used.  Use a
    // lock so 2 files don't get the same space.
    int i;	// to find the next slot

    lock->Acquire();
    i = map.Find();
    lock->Release();
    if ( i != -1){
    	table[i] = f;
        count++;
        maxCount++;
    }
    return i;
}

void *Table::Remove(int i) {
    // Remove the element associated with identifier i from the table,
    // and return it.

    void *f =0;

    if ( i >= 0 && i < size ) {
	lock->Acquire();
	if ( map.Test(i) ) {
	    map.Clear(i);
	    f = table[i];
	    table[i] = 0;
        count--;
	}
	lock->Release();
    }
    return f;
}

void Table::lockAcquire() {
    // Acquire table's lock
	lock->Acquire();
}
void Table::lockRelease() {
    // Acquire table's lock
	lock->Release();
}

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void 
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	"executable" is the file containing the object code to load into memory
//
//      It's possible to fail to fully construct the address space for
//      several reasons, including being unable to allocate memory,
//      and being unable to read key parts of the executable.
//      Incompletely consrtucted address spaces have the member
//      constructed set to false.
//----------------------------------------------------------------------

AddrSpace::AddrSpace(OpenFile *executable) : fileTable(MaxOpenFiles) {
    
    srand(time(NULL));
    
    exec = executable;
    NoffHeader noffH;
    int i;
    unsigned int size;
    int maxNumThreads = 128;
    id = processTable->Put(this);           //adds a process to the process table
    FIFOEvictionQueue = new List();
    srand(time(NULL));
    //Keep track of all of the threads that belong to the process
    threadTable = new Table(maxNumThreads);
    currentThread->setThreadTableLocation(threadTable->Put(currentThread));        //adds the thread to the thread table

    // Don't allocate the input or output to disk files
    fileTable.Put(0);
    fileTable.Put(0);

    exec->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && 
		(WordToHost(noffH.noffMagic) == NOFFMAGIC))
    	SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);

    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size ;
    DEBUG('a', "Initializing address space... size %d\n", size);
    //size of the code
    execSize = divRoundUp(size, PageSize);
    
    numPages = divRoundUp(size, PageSize) + divRoundUp(UserStackSize,PageSize);
                        // we need to increase the size
						// to leave room for the stack

    size = numPages * PageSize;     //page size is 128 bytes

    //ASSERT(numPages <= NumPhysPages);	// check we're not trying
                                        // to run anything too big --
                                        // at least until we have
                                        // virtual memory

    DEBUG('a', "Initializing address space, num pages %d, size %d\n", 
					numPages, size);
    
    pageTable = new TranslationEntryExec[numPages];

    for (i = 0; i < (int)numPages; i++)
    {
        //DEBUG('z', "AddrSpace: setting page %d invalid\n", i);
        
    	pageTable[i].valid = FALSE;
        if (i < execSize)
        {
            pageTable[i].location = EXEC;
            pageTable[i].byteOffset = noffH.code.inFileAddr + i*PageSize;
        }
        else pageTable[i].location = NEITHER;
    }
    pageTableLock = new Lock("pageTableLock");
}   

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
//
// 	Dealloate an address space.  release pages, page tables, files
// 	and file tables
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
    delete pageTable;
    delete threadTable;
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::setNewPageTable(){
    //unsigned int tempNumPages = numPages;
    //unsigned int size = noffH.code.size + noffH.initData.size + noffH.uninitData.size ;
    unsigned int previousNumPages = numPages;
    numPages += divRoundUp(UserStackSize,PageSize); //decrement on exit
    TranslationEntryExec *tempTable = new TranslationEntryExec[numPages];

    int ppn = 0;
    //copy over old Page Table data
    for (unsigned int i = 0; i < previousNumPages; i++)
    {
        //DEBUG('z', "setNewPageTable: copying page %i\n", i);
        
        tempTable[i].virtualPage = pageTable[i].virtualPage;   
        tempTable[i].physicalPage = pageTable[i].physicalPage;
        tempTable[i].valid = pageTable[i].valid;
        tempTable[i].use = pageTable[i].use;
        tempTable[i].dirty = pageTable[i].dirty;
        tempTable[i].readOnly = pageTable[i].readOnly;
        tempTable[i].location = pageTable[i].location;
        tempTable[i].byteOffset = pageTable[i].byteOffset;
    }

    for (unsigned int i = previousNumPages; i < numPages; ++i)
    {
        //DEBUG('z', "setNewPageTable: setting page %d invalid\n", ppn);
        
        tempTable[i].valid = FALSE;
        pageTable[i].location = NEITHER;
    }

    delete pageTable; 
    pageTable = tempTable;

    RestoreState();
}
//----------------------------------------------------------------------------
// The following two functions are the different eviction policies used to replace
// pages in memory. The 2 policies are Random and FIFO.
// Random Eviction will remove a random page from the IPT seeded by the runtime of the 
// process. FIFO Eviction will evict the page that has been in the IPT the longest.
//----------------------------------------------------------------------------
//Nachos -x ../test/matmult > debugFile

int AddrSpace::HandleMemoryFull()
{    
    DEBUG('z', "ENTERING HandleMemoryFull.\n");

    int pageIndex = 0;

    iptLock->Acquire();
    
   //Random Eviction
    if(evictionPolicy == 1)
    {             //default set in system.cc
        do
        {
            pageIndex = rand() % NumPhysPages;
            DEBUG('z', "HandleMemoryFull: Trying to evict page %d, use = %d\n", pageIndex, (int)ipt[pageIndex].use);
        } while (ipt[pageIndex].use);
        ipt[pageIndex].use = true;
        DEBUG('z', "HandleMemoryFull: Randomly evicted page %d from the IPT\n", pageIndex);
    }
    
    //FIFO Eviction
    else
    {
        do
        {
            pageIndex = (int) FIFOEvictionQueue->Remove();
            DEBUG('z', "HandleMemoryFull: Trying to evict page %d, use = %d\n", pageIndex, (int)ipt[pageIndex].use);
        } while (ipt[pageIndex].use);
        ipt[pageIndex].use = true;
        DEBUG('z', "HandleMemoryFull: Evicted page %d stored in the FIFO queue from the IPT\n", pageIndex);
    }
    
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    
    if (ipt[pageIndex].processID == id)
    {
        DEBUG('z', "HandleMemoryFull: Evicted page %d belongs to this process. vpn = %d\n", pageIndex, ipt[pageIndex].virtualPage);
        //look for ipt.vpn in the tlb
        //if a match, check to see if it's valid
        //if valid, propagate and set the tlb invalid
        for(int i = 0;i < TLBSize; i++)
        {
            if(machine->tlb[i].virtualPage == ipt[pageIndex].virtualPage)
            {
                if(machine->tlb[i].valid)
                {
                    DEBUG('z', "HandleMemoryFull: Propagating dirty bit of page %d to ipt page %d\n", ipt[pageIndex].virtualPage, machine->tlb[i].physicalPage);
                    ipt[machine->tlb[i].physicalPage].dirty = machine->tlb[i].dirty;
                    machine->tlb[i].valid = FALSE;
                    break;
                }
            }
        }
    }

    (void) interrupt->SetLevel(oldLevel);
    
    AddrSpace* AddrSPtemp =  (AddrSpace*)processTable->Get(ipt[pageIndex].processID);
    //If dirty is true, move to swap
    
    if(AddrSPtemp && ipt[pageIndex].dirty)
    {
        DEBUG('z', "HandleMemoryFull: Accessing swapfile\n");
        //write to swapfile
        if(AddrSPtemp->pageTable[ipt[pageIndex].virtualPage].location == SWAP)
        {
            DEBUG('z', "  HandleMemoryFull: Page from page table is already in the swapfile\n");
            swapFile->WriteAt(&(machine->mainMemory[pageIndex * PageSize]), PageSize, AddrSPtemp->pageTable[ipt[pageIndex].virtualPage].byteOffset);
            
            DEBUG('z', "HandleMemoryFull: Page written to same swap. vpn = %d, ppn = %d, byteOffset = %d\n", AddrSPtemp->pageTable[ipt[pageIndex].virtualPage].virtualPage, AddrSPtemp->pageTable[ipt[pageIndex].virtualPage].physicalPage, AddrSPtemp->pageTable[ipt[pageIndex].virtualPage].byteOffset);
        }

        else
        {
            int sf = swapFileMap->Find();
            if(sf != -1)
            {
                
                swapFile->WriteAt(&(machine->mainMemory[pageIndex * PageSize]), PageSize, PageSize*sf);
                AddrSPtemp->pageTable[ipt[pageIndex].virtualPage].location = SWAP;
                AddrSPtemp->pageTable[ipt[pageIndex].virtualPage].byteOffset = PageSize*sf;
               
                DEBUG('z', "HandleMemoryFull: Page written to new swap pos. vpn = %d, ppn = %d, byteOffset = %d\n", AddrSPtemp->pageTable[ipt[pageIndex].virtualPage].virtualPage, AddrSPtemp->pageTable[ipt[pageIndex].virtualPage].physicalPage, AddrSPtemp->pageTable[ipt[pageIndex].virtualPage].byteOffset);

            }
            else
                DEBUG('z', "HandleMemoryFull: swapfile full!!\n");
        }
    }

    ipt[pageIndex].use = FALSE;
    
    iptLock->Release();

    DEBUG('z', "EXITING HandleMemoryFull. ppn = %d\n", pageIndex);
    
    return pageIndex;
}
int AddrSpace::HandleIPTMiss(int vpn)
{
    int ppn = getFreePage();
    
    DEBUG('z', "ENTERING HandleIPTMiss. vpn = %d, ppn = %d\n", vpn, ppn);
    
    // if ppn = -1, kick a page
    if(ppn == -1)
        ppn = HandleMemoryFull();
    
    pageTableLock->Acquire();
    
    // if vpn is not stack
    //  copy from executable
    if (pageTable[vpn].location == EXEC)
    {
        DEBUG('a', "HandleIPTMiss: Reading from executable file. vpn = %d, ppn = %d, byteOffset = %d\n", vpn, ppn, pageTable[vpn].byteOffset);
        
        exec->ReadAt(&(machine->mainMemory[ppn * PageSize]), PageSize, pageTable[vpn].byteOffset);
        
        pageTable[vpn].location = EXEC; // should already be set, but just in case
    }
    
    //look through the page table and if it is in the swap file then read it back in
    if (pageTable[vpn].location == SWAP)
    {
        DEBUG('z', "HandleIPTMiss: Reading from swap file. vpn = %d, ppn = %d, byteOffset is %d\n", vpn, ppn, pageTable[vpn].byteOffset);
        
        swapFile->ReadAt(&(machine->mainMemory[ppn * PageSize]), PageSize, pageTable[vpn].byteOffset);
    }
    
    //DEBUG('z', "HandleIPTMiss: setting virtual page %d valid\n", vpn);
    
    pageTable[vpn].virtualPage = vpn;   
    pageTable[vpn].physicalPage = ppn;
    pageTable[vpn].valid = TRUE;
    pageTable[vpn].use = FALSE;
    pageTable[vpn].dirty = FALSE;
    pageTable[vpn].readOnly = FALSE;
    
    pageTableLock->Release();
    
    iptLock->Acquire();
    
    //DEBUG('z', "HandleIPTMiss: setting physical page %d valid\n", ppn);
    
    ipt[ppn].virtualPage = vpn;
    ipt[ppn].physicalPage = ppn;
    ipt[ppn].space = this;
    ipt[ppn].processID = id;
    ipt[ppn].valid = TRUE;
    ipt[ppn].use = FALSE;
    ipt[ppn].dirty = FALSE;
    ipt[ppn].readOnly = FALSE;

    DEBUG('z', "HandleIPTMiss: Page loaded to ipt. vpn = %d, ppn = %d, byteOffset = %d\n", ipt[ppn].virtualPage, ipt[ppn].physicalPage, pageTable[vpn].byteOffset);

    // add ppn to the FIFO queue
    FIFOEvictionQueue->Append((void*)ppn);

    iptLock->Release();
    
    DEBUG('z', "EXITING HandleIPTMiss. ppn = %d\n", ppn);
    
    return ppn;
}
//Copy page table info to the tlb
void AddrSpace::PageFault()
{    
    //page table index
    int regVal = (int)machine->ReadRegister(39);
    int PTIndex = getPPN(regVal/PageSize); // will return -1 if not found
    //DEBUG('z', "PageFault: reg = %d, vpn = %d, ppn = %d\n", regVal, regVal/PageSize, PTIndex);

    if (PTIndex == -1)
    {
        PTIndex = HandleIPTMiss(regVal/PageSize);
    }
    
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    
    //DEBUG('z', "PageFault: copying ppn %d to tlb %d\n", PTIndex, currentTLB);

    if (machine->tlb[currentTLB].valid)
    {
        ipt[machine->tlb[currentTLB].physicalPage].dirty = machine->tlb[currentTLB].dirty;
    }
    
    machine->tlb[currentTLB].virtualPage = ipt[PTIndex].virtualPage;
    machine->tlb[currentTLB].physicalPage = ipt[PTIndex].physicalPage;
    machine->tlb[currentTLB].valid = ipt[PTIndex].valid;
    machine->tlb[currentTLB].use = ipt[PTIndex].use;
    machine->tlb[currentTLB].dirty = ipt[PTIndex].dirty; 
    machine->tlb[currentTLB].readOnly = ipt[PTIndex].readOnly;
    
    if(currentTLB >= TLBSize - 1)
        currentTLB = 0;
    else
        currentTLB++;
    
    (void) interrupt->SetLevel(oldLevel);
}

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %x\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState() {}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//  This will also invalitdate the TLB
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    //DEBUG('z', "RestoreState has been called\n");
    machine->pageTableSize = numPages;
    //invalidate the TLB
    for(int i = 0; i <TLBSize; i++)
    {
        //DEBUG('z', "RestoreState: setting tlb page %d invalid\n", i);

        if(machine->tlb[i].virtualPage == ipt[machine->tlb[i].physicalPage].virtualPage){
            if(machine->tlb[i].valid){
                DEBUG('p', "ipt index on context switch is %d\n", machine->tlb[i].physicalPage);
                ipt[machine->tlb[i].physicalPage].dirty = machine->tlb[i].dirty;
            }
        }
        machine->tlb[i].valid = false;
    }
    (void) interrupt->SetLevel(oldLevel);  
}

int AddrSpace::getPPN(int vpn)
{
    iptLock->Acquire();
    for (int i = 0; i < NumPhysPages; i++)
    {
        TranslationEntryIPT t = ipt[i];
        //DEBUG('z', "i = %d, valid = %d, processID = %d, id = %d, virtualPage = %d, vpn = %d, physicalPage = %d\n", i, (int)t.valid, t.processID, id, t.virtualPage, vpn, t.physicalPage);
        if (t.valid && t.processID == id && t.virtualPage == vpn)
        {
            iptLock->Release();
            return t.physicalPage;
        }
    }
    iptLock->Release();
    return -1;
}

int AddrSpace::getFreePage()
{
    iptLock->Acquire();
    for (int i = 0; i < NumPhysPages; i++)
    {
        TranslationEntryIPT t = ipt[i];
        if (! t.valid)
        {
            iptLock->Release();
            return t.physicalPage;
        }
    }
    iptLock->Release();
    return -1;
}
