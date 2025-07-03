
#include "DataBlockNode.h"


void DataBlockNode::addNode(DataBlockNode* someNode) {
	someNode->headNode = &*this->headNode;
	someNode->nextNode = this->nextNode;
	someNode->prevNode = this;
	this->nextNode = someNode;
	someNode->nextNode->prevNode = someNode;
};

void DataBlockNode::addDataBlock(DataBlock* someBlock) {
	DataBlockNode* temp = new DataBlockNode(someBlock);
	temp->blockOfData->nodeList = NodeList();
	temp->blockOfData->nodeList.memArenaList = temp;
	addNode(temp);
};

void DataBlockNode::removeNode() {
	this->nextNode->prevNode = this->prevNode;
	this->prevNode->nextNode = this->nextNode;
};

void DataBlockNode::removeNodeList() {
	this->blockOfData->nodeList.removeNodeFromAllLists();
};

DataBlockNode* DataBlockNode::findNode(DataBlockCodeType someCode) {
	return headNode->findNode(someCode, false);
};

DataBlock* DataBlockNode::findSpaceForAllocation(DataBlockSizeType blockSize) {
	return headNode->findSpaceForAllocation(blockSize, false);
};

void DataBlockNode::destroyAllNodes() {
	if (headNode != nextNode) {
		nextNode->destroyAllNodes();
		removeNodeList();
		delete blockOfData;
		delete this;
	}
}

DataBlockNode* DataBlockNode::findNode(DataBlockCodeType someCode, bool headNodeFound) {
	if (this->blockOfData->blockCode == someCode) {
		return this;
	}
	if (this == headNode && false == headNodeFound) {
		return this->nextNode->findNode(someCode, true);
	}
	if (this == headNode && true == headNodeFound) {
		return nullptr;
	}
	return this->nextNode->findNode(someCode, headNodeFound);

};

DataBlock* DataBlockNode::findSpaceForAllocation(DataBlockSizeType blockSize, bool startedFromHead) {
	if (this->blockOfData->blockCode == 0 && this->blockOfData->blockSize >= blockSize) {
		return this->blockOfData;
	}
	if (this == headNode && false == startedFromHead) {
		return this->nextNode->findSpaceForAllocation(blockSize, true);
	}
	if (this == headNode && true == startedFromHead) {
		return nullptr;
	}
	return this->nextNode->findSpaceForAllocation(blockSize, startedFromHead);

};
