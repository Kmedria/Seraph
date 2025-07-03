#pragma once
#include "DataBlockNode.h"

// Stores and manages data by using blocks and linked lists.
class MemoryArena {
private:
	// it is assumed that the arena will not hold more than 65 kb, so short is used for arena size.
	
	// counter that increases for every new datablock, an easy short term solution for a unique code.
	DataBlockCodeType counter = 1;

	void* m_arena; // Starting location of the Arena
	DataBlockSizeType m_arenaSize = 1; // Arena size in bytes
	
	DataBlockNode* m_dataMap; // an array that maps the location of data (using blocks) to a given block code

	// adjusts the size of the Arena by multiplying size with inputted ratio
	void rebuildArena(float sizeRatio); 
	
	// ideally the arrays will be kept to a size such that 63%-86% of the array is in use (those are arbitary numbers I chose, 1-e^-1 and 1-e^-2) 

public: 

	MemoryArena();
	MemoryArena(unsigned short initSize);
	MemoryArena& operator=(MemoryArena&& rhs);

	~MemoryArena(); // deletes the Arena and Data block arrays and clears any other neccessary resources

	DataBlockSizeType getSize() {
		return m_arenaSize;
	};


	// below func should be deleted after testing is complete
	void* getAddress() {
		return m_arena;
	};


	// checks to see if there is enough space to add a datablock of the given size to the dataMap
	bool isAvailable(DataBlockSizeType dataBlockSize);

	//__int8 addInitialData(unsigned short dataSizeInBytes); // should be used when adding initial data (same as allocateData(), but does not shrink the array)

	DataBlockCodeType allocateData(DataBlockSizeType dataSizeInBytes); // finds a empty space big enough for the data and returns its assigned unique code
	void deallocateData(DataBlockCodeType dataBlockCode); // finds the data block assossicated with the code and removes it (does not manage memory by deleting object so caller must use delete if new was used)

	void* getDataAddress(DataBlockCodeType dataBlockCode); // Retrieves the memory address of the data block with input code, but the object is not deallocated (rather use pop).
	void* getBlockAddress(DataBlockCodeType dataBlockCode); // Retrieves the memory address of the data block with input code, but the object is not deallocated (rather use pop).

};