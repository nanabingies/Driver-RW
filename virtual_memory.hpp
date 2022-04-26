#pragma once

#include <Windows.h>
#include <iostream>
#include <Psapi.h>
#include "memory.hpp"

namespace virtual_memory {
	uintptr_t convert_virtual_to_physical(HANDLE driver, uintptr_t pml4Base, uintptr_t virtual_address);

	uintptr_t NtoskrnlBaseAddress();

	uintptr_t convert(HANDLE driver, uintptr_t pml4Base, uintptr_t virtual_address);

	uintptr_t GetPml4Base(HANDLE driver);
}