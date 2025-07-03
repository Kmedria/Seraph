#include "MemoryArena.h"
#include "CommonIncludes.h"

int log_2(int inputValue) {
	// returns the rounded-up answer of log(x) base 2
	int counter = 0;
	int outputValue = 1;
	while (outputValue < inputValue) {
		counter++;
		outputValue *= 2;
	}
	return counter;
}

MemoryArena::MemoryArena() {
	MemoryArena(2);
}

MemoryArena::MemoryArena(unsigned short initialSize) {
	m_arenaSize = initialSize;
	m_arena = new char[m_arenaSize];
	m_dataMap = new DataBlockNode();
	m_dataMap->blockOfData = new DataBlock(initialSize, m_dataMap);
}

MemoryArena& MemoryArena::operator=(MemoryArena&& rhs) {
	if (this != &rhs) {

		this->counter = rhs.counter;
		this->m_arenaSize = rhs.m_arenaSize;

		if (this->m_arena != nullptr) {
			delete[] this->m_arena;
			this->m_arena = nullptr;
		}
		if (this->m_dataMap != nullptr) {
			this->m_dataMap = m_dataMap->headNode;
			this->m_dataMap->destroyAllNodes();
			this->m_dataMap = nullptr;
		}
		if (rhs.m_arena != nullptr) {
			this->m_arena = rhs.m_arena;
			rhs.m_arena = nullptr;
		}
		if (rhs.m_dataMap != nullptr) {
			this->m_dataMap = rhs.m_dataMap;
			rhs.m_dataMap = nullptr;
		}
	}
	return *this;
}

MemoryArena::~MemoryArena() {
	if (m_arena != nullptr) {
		delete[] m_arena;
		m_arena = nullptr;
	}
	if (m_dataMap != nullptr) {
		m_dataMap = m_dataMap->headNode;
		m_dataMap->destroyAllNodes();
		m_dataMap = nullptr;
	}
}

/*
* Changes the size of the array by multipying with some ratio.
  This function is only called to try to keep 50%-75% of the array in use. This will ensure that the array does not get uneccassarily large with little data and also tries to keep space for new data.
*/
void MemoryArena::rebuildArena(float sizeRatio) {
	DataBlockSizeType newSize = DataBlockSizeType(m_arenaSize * sizeRatio);
	void* tempArena = new char[newSize];
	int min = newSize > m_arenaSize? m_arenaSize : newSize;
	for (int index = 0; index < min; index++) {
		((char*)tempArena)[index] = ((char*)m_arena)[index];
	}
	
	delete[] m_arena;

	m_arenaSize = newSize;
	m_arena = tempArena;
};

/*
* Takes the input and finds an empty spot in the Arena big enough and returns the dataBlock ID assigned to that space
*/
DataBlockCodeType MemoryArena::allocateData(DataBlockSizeType dataSizeInBytes)
{
	DataBlock* space = (*m_dataMap).findSpaceForAllocation(dataSizeInBytes);

	switch (space == nullptr) {
	case true:
		rebuildArena(2);
		return allocateData(dataSizeInBytes);
		break;
	case false:
		switch (space->blockSize > dataSizeInBytes) {
		case true:
		{
			DataBlock* add = new DataBlock(counter++, dataSizeInBytes);
			add->blockOffset = space->blockOffset + space->blockSize - dataSizeInBytes;

			space->nodeList.memArenaList->addDataBlock(add);
			space->blockSize -= dataSizeInBytes;
			break;
		}
		case false:
		{
			space->blockCode = counter++;
			break;
		}
		}
		return counter - 1;
	}
}

bool MemoryArena::isAvailable(DataBlockSizeType dataBlockSize) {
	return nullptr != (*m_dataMap).findSpaceForAllocation(dataBlockSize);
}

/*
* Takes the input code and finds the datablock in the Arena removes it from the arena. It also removes the dataBlock from all linked lists.
*/
void MemoryArena::deallocateData(DataBlockCodeType dataCode) {
	DataBlockNode* node = (*m_dataMap).findNode(dataCode);

	if (nullptr != node) {
		DataBlockNode* prev = node->prevNode;
		DataBlockNode* next = node->nextNode;

		if (node->blockOfData->blockOffset > 0 && nullptr != prev && 0 == prev->blockOfData->blockCode) {
			node->removeNodeList();
			prev->blockOfData->blockSize += node->blockOfData->blockSize;
			if (nullptr != next && next->blockOfData->blockOffset > node->blockOfData->blockOffset && 0 == next->blockOfData->blockCode) {
				prev->blockOfData->blockSize += next->blockOfData->blockSize;
				next->removeNodeList();
				delete next->blockOfData;
				delete next;
			}
			delete node->blockOfData;
			delete node;
		} else if (nullptr != next && next->blockOfData->blockOffset > node->blockOfData->blockOffset && 0 == next->blockOfData->blockCode) {
			node->removeNodeList();
			next->blockOfData->blockOffset = node->blockOfData->blockOffset;
			next->blockOfData->blockSize += node->blockOfData->blockSize;
	
			delete node->blockOfData;
			delete node;
		} else {
			node->blockOfData->blockCode = 0;
		}
	}
}

void* MemoryArena::getDataAddress(DataBlockCodeType dataBlockCode) {

	DataBlockNode* temp = (*m_dataMap).findNode(dataBlockCode);
	if (temp == nullptr) {
		return nullptr;
	}
	return (char*)m_arena + temp->blockOfData->blockOffset;
}

void* MemoryArena::getBlockAddress(DataBlockCodeType dataBlockCode) {

	DataBlockNode* temp = (*m_dataMap).findNode(dataBlockCode);
	if (temp == nullptr) {
		return nullptr;
	}
	return temp->blockOfData;
}
