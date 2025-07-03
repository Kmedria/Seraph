#include "DataBlock.h"

#include "DataBlockNode.h"

NodeList::NodeList() {}


NodeList::NodeList(DataBlockNode* memNode) {
	memArenaList = memNode;
}

NodeList::~NodeList() {}


unsigned __int8 NodeList::nodeListCount() {

	unsigned __int8 count = 0;

	if (nullptr != memArenaList) {
		count++;
	}

	return count;

}

void NodeList::removeNodeFromAllLists() {
	
	memArenaList->removeNode();
	// if there were other nodes in the list, they would be removed and then deleted.
	
}

