#pragma once

#include "DataBlock.h"
#include "DataBlockTypes.h"

// Circular linked list (intrusive) of datablocks
class DataBlockNode {
public:

	DataBlockNode* headNode = nullptr;
	DataBlockNode* prevNode = nullptr;
	DataBlockNode* nextNode = nullptr;

	DataBlock* blockOfData = nullptr;

	// This null constructor should not be used.
	DataBlockNode() : headNode(this), prevNode(this), nextNode(this), blockOfData(nullptr) {}

	// This constructor should only be used for the very first Node.
	DataBlockNode(DataBlockSizeType initSize) : headNode(this), prevNode(this), nextNode(this) {
		blockOfData = new DataBlock(initSize, this);
	}

	// 
	DataBlockNode(DataBlock* someBlock) : headNode(this), prevNode(this), nextNode(this), blockOfData(someBlock) {}

	// Deconstructor deletes the object as it is no longer needed.
	~DataBlockNode() {
	}

	// adds new dataBlock in a new node after the current node in the list
	void addDataBlock(DataBlock* someBlock);

	// adds new node after the current node in the list
	void addNode(DataBlockNode* someNode);

	// removes the current node from the list (doesn't remove the data from memory)
	void removeNode();

	// removes the current node from the list (doesn't remove the data from memory)
	void removeNodeList();

	// finds the Node with the input code, returns nullptr if not found
	DataBlockNode* findNode(DataBlockCodeType someCode);

	// Searches the linked list for a data block of appropriate size
	DataBlock* findSpaceForAllocation(DataBlockSizeType blockSize);

	void destroyAllNodes();

private:

	// finds the Node with the input code, returns nullptr if not found
	DataBlockNode* findNode(DataBlockCodeType someCode, bool headNodeFound);

	// Searches the linked list for a data block of appropriate size, return nullptr if not found
	DataBlock* findSpaceForAllocation(DataBlockSizeType blockSize, bool startedFromHead);

};