#pragma once
struct object {};

class MemoryArena {
private:
	int x;
public:

	MemoryArena();
	~MemoryArena();

	int addItem();
	void removeItem();

};