#pragma once

#include <Windows.h>
#include <iostream>

namespace memory {
	uintptr_t map_physical_memory(HANDLE driver, uintptr_t physical_address, unsigned long size);
	uintptr_t unmap_physical_memory(HANDLE driver, uintptr_t physical_address);

	bool read_physical_memory(HANDLE driver, uintptr_t physical_memory, void* buffer,
		unsigned long size);
	bool write_physical_memory(HANDLE driver, uintptr_t physical_memory, void* buffer,
		unsigned long size);
}