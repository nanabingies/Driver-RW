#include "virtual_memory.hpp"

namespace virtual_memory {
	uintptr_t convert_virtual_to_physical(HANDLE driver, uintptr_t pml4Base, uintptr_t virtual_address) {
		WORD pml4Offset;
		uintptr_t PML4E;
		WORD pdptOffset;
		uintptr_t PDPE;
		WORD pdtOffset;
		uintptr_t PDE;
		WORD ptOffset;
		uintptr_t PTE;
		WORD phyPageOffset;
		uintptr_t physicalAddress;

		//std::cout << "[+] Virtual Address : 0x" << std::hex << virtual_address << std::endl;

		// Get PML4 offset from virtual address
		pml4Offset = (virtual_address >> (12 + 9 + 9 + 9)) & 0x1FF;
		//std::cout << "[+] PML4 Offset : " << std::hex << pml4Offset << std::endl;

		// Get PML4 / PXE
		if (!memory::read_physical_memory(driver, (pml4Base + (pml4Offset * 8)), &PML4E, sizeof(PML4E))) {
			std::cout << "[-] Unable to read physical memory to get PML4E/PXE!\n";
			return NULL;
		}

		//std::cout << "[+] PML4E : 0x" << std::hex << PML4E << std::endl;

		// Get PDPT offset from virtual address
		pdptOffset = (virtual_address >> (12 + 9 + 9)) & 0x1FF;
		//std::cout << "[+] PDPT offset : " << std::hex << pdptOffset << std::endl;

		// Get PDPE / PPE
		if (!memory::read_physical_memory(driver, ((PML4E & 0xFFFFFFFFFF000) + (pdptOffset * 8)), &PDPE, sizeof(PDPE))) {
			std::cout << "[-] Unable to read physical memory to get PDPE/PPE!\n";
			return NULL;
		}

		//std::cout << "[+] PDPE : 0x" << std::hex << PDPE << std::endl;

		if (PDPE == 0)
			return NULL;

		// Get PDT offset from virtual address
		pdtOffset = (virtual_address >> (12 + 9)) & 0x1FF;
		//std::cout << "[+] PDT offset : " << std::hex << pdtOffset << std::endl;

		// Get PDE
		if (!memory::read_physical_memory(driver, ((PDPE & 0xFFFFFFFFFF000) + (pdtOffset * 8)), &PDE, sizeof(PDE))) {
			std::cout << "[-] Unable to read physical memory to get PDE!\n";
			return NULL;
		}

		//std::cout << "[+] PDE : " << std::hex << PDE << std::endl;

		if (PDE == 0)
			return NULL;

		// Check for 2MB pages - LargePage(L) bit set in PDE
		// Entry page size bit = 0x80
		// Ref: https://www.cnblogs.com/bianchengnan/p/6231597.html
		if ((PDE & 0x80) != 0) {
			// Physical address mask for 2 MB pages = 0xFFFFFFFE00000
			// Physical page offset for 2 MB pages = 0x1FFFFF
			physicalAddress = ((PDE & 0xFFFFFFFE00000) + (virtual_address & 0x1FFFFF));
			std::cout << "[+] Physical Address: 0x " << std::hex << physicalAddress << std::endl;
			return (uintptr_t)physicalAddress;
		}

		// Get PT offset from virtual address
		ptOffset = (virtual_address >> 12) & 0x1FF;
		//std::cout << "[+] PT offset : " << std::hex << ptOffset << std::endl;

		// Get PTE
		if (!memory::read_physical_memory(driver, ((PDE & 0xFFFFFFFFFF000) + (ptOffset * 8)), &PTE, sizeof(PTE))) {
			std::cout << "[-] Unable to read physical memory to get PTE!\n";
			return NULL;
		}

		//std::cout << "[+] PTE : 0x" << std::hex << PTE << std::endl;

		if (PTE == 0)
			return NULL;

		// Get physical page offset from virtual address
		// Physical page offset for 4 kB pages = 0xFFF
		phyPageOffset = virtual_address & 0xFFF;
		//std::cout << "[+] Physical page offset : " << std::hex << phyPageOffset << std::endl;

		// Calculate physical address
		// Physical address mask for 4 kB pages = 0xFFFFFFFFFF000
		physicalAddress = (PTE & 0xFFFFFFFFFF000) + phyPageOffset;
		//std::cout << "[+] Physical Address : 0x" << std::hex << physicalAddress << std::endl;

		return (uintptr_t)physicalAddress;
	}

	uintptr_t NtoskrnlBaseAddress() {
		LPVOID drivers[1024] = { 0 };
		DWORD lpcbNeeded = 0x0;
		TCHAR driverNames[1024];

		if (!EnumDeviceDrivers(drivers, sizeof(drivers), &lpcbNeeded)) {
			std::cout << "[-] Failed to get base addresses of kernel modules.\n";
			return NULL;
		}

		/*for (int i = 0; i < lpcbNeeded / sizeof(drivers[0]); i++) {
			if (GetDeviceDriverBaseName(drivers[i], driverNames, sizeof(driverNames))) {
				std::cout << "[+] module name " << (LPSTR)driverNames << "\tBase Address : 0x" << std::hex << drivers[i] << std::endl;
				if (!strcmp((const char*)driverNames, "win32k")) {
					std::cout << "[+] Got ntoskrnl.exe base address : 0x" << std::hex << drivers[i] << std::endl;
					return reinterpret_cast<uintptr_t>(drivers[i]);
				}
			}
		}*/

		return reinterpret_cast<uintptr_t>(drivers[0]);
	}

	// https://public.cnotools.studio/bring-your-own-vulnerable-kernel-driver-byovkd/utilities/leaking-kernel-dtb
	uintptr_t GetPml4Base(HANDLE driver) {
		DWORD offset = 0x1000; 
		DWORD limit = 0x100000; 
		uintptr_t buffer = NULL;
		WORD cr3Offset = 0xa0; 
		uintptr_t pml4Base = NULL;

		// Loop until we find _PROCESSOR_START_BLOCK PA by heuristic scanning and get CR3 value
		while (offset < limit) {
			if (!memory::read_physical_memory(driver, offset, &buffer, sizeof(uintptr_t))) {
				std::cout << "[-] Unable to read physical memory to get _PROCESSOR_START_BLOCK PA!\n";
				return NULL;
			}
			//std::cout << "\tOffset : " << std::hex << offset << "\tBuffer : " << std::hex << buffer << std::endl;
			BYTE c = buffer & 0xff;
			
			if (c == 0xe9){ // jmp
				
				if (!memory::read_physical_memory(driver, (offset + cr3Offset), &pml4Base, sizeof(uintptr_t))) {
					std::cout << "[-] Unable to read Cr3 value\n";
					return NULL;
				}
				std::cout << "[+] Cr3 : " << std::hex << pml4Base << std::endl;
				break;
			}

			offset += 0x1000;
		}
		return pml4Base;
	}
}
