#pragma once

#include "virtual_memory.hpp"

namespace process{
	inline uintptr_t pml4Base = 0x0; //0x00187000;
	inline uintptr_t converted_address = 0x0;
	inline uintptr_t buffer;
	inline uintptr_t read_data;

	// Offsets according to Windows Versions
	const DWORD64 UniqueProcessIdOffset = 0x2e8; //0x180;
	const DWORD64 ActiveProcessLinksOffset = 0x2f0; //0x188;
	const DWORD64 TokenOffset = 0x358; // 0x208;

	uintptr_t PsGetCurrentProcess(HANDLE driver);

	uintptr_t PsInitialSystemProcess(HANDLE driver);
}