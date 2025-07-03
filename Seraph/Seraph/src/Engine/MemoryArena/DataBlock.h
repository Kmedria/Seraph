#pragma once

#include "DataBlockTypes.h"

class DataBlockNode;

// Nodelist Stores the location of a given datablock in each DataBlockNodes linked list.
struct NodeList {
public:

	DataBlockNode* memArenaList = nullptr; // The location/position where the data is actually stored. This must have a value for all iterations.

	// Null constructor that leaves all nodes as nullptr, should not be used since member variable memArenaList is neccessary.
	NodeList();

	// The constructor that should be used as it requires all the neccassary values.
	NodeList(DataBlockNode* memNode);

	// Deconstructor, no special purpose, for now.
	~NodeList();

	// Counts the number of nodes that have some non-null value. 
	// The count should be at least 1, as every dataBlock should be in the memArenaList 
	// (since it stores the actual data location of the datablock.).
	unsigned __int8 nodeListCount();

	// removes all the nodes from all linked lists and deletes them.
	void removeNodeFromAllLists();
};

// A dataBlock maps a block of data that stores the size and current use of that block. 
// It is only a record and doesn't actually allocate/use that data, but it is used by the Memory Arena to do such actions.  
// Each block should ideally store 1 large object or multiple smaller objects that are used together (such as particles that all follow the same rules).
// The Block code variable allows us to uniquely identify each dataBlock. If the block code = 0, the block is empty (unused), otherwise the number points a specific datablock.
struct DataBlock {
public:

	DataBlockCodeType blockCode = 0; // stores a unique code (of type DataBlockCodeType right now) that identifies its specific data block
	DataBlockSizeType blockSize = 1; // stores the size of the data block (in bytes)

	DataBlockSizeType blockOffset = 0;

	NodeList nodeList; // This nodes stores the block's current location in each node list and shows whether a block is in a given list or not.

	// This constructor shouldn't be used generally as byte sized data blocks are inefficient if too many are required.
	DataBlock() : blockCode(0), blockSize(1) {
		nodeList = NodeList();
	}

	// Create an unused datablock of some size given by the someSize variable
	DataBlock(DataBlockSizeType someSize) : blockCode(0), blockSize(someSize), blockOffset(0) {
		nodeList = NodeList();
	}

	// Create an unused datablock of some size given by the someSize variable
	DataBlock(DataBlockSizeType someSize, DataBlockNode* someNode) :
		blockCode(0), blockSize(someSize), blockOffset(0), nodeList(NodeList(someNode)) {}

	// Create an unused datablock of some size given by the someSize variable
	DataBlock(DataBlockSizeType someSize, DataBlockSizeType someoffset, DataBlockNode* someNode) :
		blockCode(0), blockSize(someSize), blockOffset(someoffset), nodeList(NodeList(someNode)) {}

	// Create an unused datablock of some size given by the someSize variable
	DataBlock(DataBlockCodeType someCode, DataBlockSizeType someSize) :
		blockCode(someCode), blockSize(someSize), blockOffset(0), nodeList(NodeList()) {}

	DataBlock(DataBlockCodeType someCode, DataBlockSizeType someSize, DataBlockSizeType someOffset, DataBlockNode* someNode) :
		blockCode(someCode), blockSize(someSize), blockOffset(someOffset), nodeList(NodeList(someNode)) {}

	// Deconstructor with no special purpose
	~DataBlock() {

	};

	// returns the number of linked lists that this dataBlock is in (minimum is 1).
	unsigned __int8 nodeListCount() {
		return nodeList.nodeListCount();;
	}
};