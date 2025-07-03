
#include "MemoryArena.h"
#include <string>
#include <iostream>
#include "_TEST_MemArena.h"


int f() {

	std::string s;
	
	bool keepRunning = true;

	MemoryArena mem;

	bool set = false;

	while (keepRunning) {
		std::getline(std::cin, s);

		if ("exit" == s) {
			keepRunning = false;
			continue;
		}
		if ("print" == s) {
			if (!set) {
				std::cout << " You cannot print when you have not initialised." << std::endl;
				continue;
			} else {
				int size = mem.getSize();
				void* address = mem.getAddress();
				char* temp;
				std::cout << std::endl;
				for (int index = 0; index < size; index++) {
					temp = (char*)address + index;
					std::cout << *temp;
				}
				std::cout << std::endl;
			}
		}
		if ("init" == s) { // init the memArena
			int size;
			std::cout << "Enter Arena Size\n" << std::endl;
			std::cin >> size;
			mem = MemoryArena(size);
			set = true;
		}
		if ("all" == s) { // allocate Data
			if (set) {
				std::cout << "What Size allocation" << std::endl;
				int size;
				std::cin >> size;

				std::cout << std::endl << (int)mem.allocateData(size) << std::endl;
			}
		}
		if ("edit" == s) { // deallocate data
			if (set) {
				std::cout << "Enter Block Code" << std::endl;
				DataBlockCodeType code;
				std::cin >> code;

				DataBlock* temp = (DataBlock*)mem.getBlockAddress(code);

				if (nullptr == temp) {
					std::cout << "This code does not occur in the arena" << std::endl;
				}
				else {

					std::cout << "You may enter " << temp->blockSize << " characters in. Any extra charaters will be ignored" << std::endl;
					std::string text;
					std::cin >> text;

					void* address = temp->blockOffset + (char*)mem.getAddress();
					char* tempAddress;

					for (int index = 0; index < (temp->blockSize); index++) {
						tempAddress = (char*)address + index;
						if (text.length() > index) {
							*tempAddress = text[index];
						}
						else {
							*tempAddress = ' ';
						}
					}
				}
			}
		}
		if ("del" == s) { // deallocate data
			if (set) {
				std::cout << "Enter Block Code" << std::endl;
				DataBlockCodeType code;
				std::cin >> code;

				if (nullptr != mem.getBlockAddress(code)) {
					mem.deallocateData(code);
				}
			}
		}
	}

	return 0;
}