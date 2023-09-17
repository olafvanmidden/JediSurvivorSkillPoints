// Includes
#include <iostream>
#include <Windows.h>
#include <vector>
#include <TlHelp32.h>

// Gets processID from processName
uintptr_t GetProcessID(const char *processName) {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) {
        return 0;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(hSnap, &pe32)) {
        CloseHandle(hSnap);
        return 0;
    }

    do {
        if (_stricmp(processName, pe32.szExeFile) == 0) {
            CloseHandle(hSnap);
            return pe32.th32ProcessID;
        }
    } while (Process32Next(hSnap, &pe32));

    CloseHandle(hSnap);
    return 0;
}

// Gets baseAddress from processID
uintptr_t GetBaseAddress(uintptr_t processID) {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, processID);
    if (hSnap == INVALID_HANDLE_VALUE) {
        return 0;
    }

    MODULEENTRY32 me32;
    me32.dwSize = sizeof(MODULEENTRY32);
    if (!Module32First(hSnap, &me32)) {
        CloseHandle(hSnap);
        return 0;
    }

    CloseHandle(hSnap);
    return (uintptr_t) me32.modBaseAddr;
}

int main() {

    std::cout << "Make sure game is running and program is started as administrator!" << std::endl;

    // Define the target process ID using GetProcessID
    uintptr_t processID = GetProcessID("JediSurvivor.exe");

    // Open the target process with required access rights
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
    if (hProcess == NULL) {
        std::cerr << "Failed to open process. Error code: " << GetLastError() << std::endl;
        system("pause");
        return 1;
    }

    // Get base address of the process using GetBaseAddress
    uintptr_t baseAddress = GetBaseAddress(processID);
    if (baseAddress == 0) {
        std::cerr << "Couldn't get base address." << std::endl;
        system("pause");
        return 1;
    }

    // Define startPointer
    uintptr_t startPointer = 0x063F1380;

    // Define the dynamic address of the pointer chain in the target process by combining the baseAdress and startPointer
    uintptr_t dynamicAddress = baseAddress + startPointer;

    // Define the offsets to follow in the pointer chain
    std::vector<int> pointerOffsets = {0x0, 0x100, 0x20, 0xA8, 0xF0, 0x428, 0x164};

    // Define a variable to store the final value in final address
    int finalAddressValue = 0;

    // Follow the pointer chain to access the final value
    uintptr_t currentAddress = dynamicAddress;

    for (const int offset: pointerOffsets) {
        if (!ReadProcessMemory(hProcess, (LPVOID) currentAddress, &currentAddress, sizeof(currentAddress), NULL)) {
            std::cerr << "Failed to read pointer in the chain. Error code: " << GetLastError() << std::endl;
            CloseHandle(hProcess);
            system("pause");
            return 1;
        }

        // Apply the offset to get the next address in the chain
        currentAddress += offset;
    }

    // Read the final value
    if (!ReadProcessMemory(hProcess, (LPVOID) currentAddress, &finalAddressValue, sizeof(finalAddressValue), NULL)) {
        std::cerr << "Failed to read process memory. Error code: " << GetLastError() << std::endl;
        CloseHandle(hProcess);
        system("pause");
        return 1;
    }

//    std::cout << "Final value at address " << std::hex << currentAddress << " is " << std::dec << finalAddressValue << std::endl;
    std::cout << "Current skillpoint value is: " << std::dec << finalAddressValue << "\n" << std::endl;

    // Define the new value you want to write to the final value address
    int newValue;

    // Add an input to get the amount of skill points the user wants
    std::cout << "Enter the amount of skill points you want:";
    std::cin >> newValue;

    // Write the new value to the final address
    if (!WriteProcessMemory(hProcess, (LPVOID) currentAddress, &newValue, sizeof(newValue), NULL)) {
        std::cerr << "Failed to write process memory. Error code: " << GetLastError() << std::endl;
        CloseHandle(hProcess);
        system("pause");
        return 1;
    }

//    std::cout << "Value at address " << std::hex << currentAddress << " changed to " << std::dec << newValue << std::endl;
    std::cout << "\nDone!\nSkillpoint value changed to: " << std::dec << newValue << std::endl;

    std::cout << "\nCreated by: Olaf van Midden\n" << std::endl;

    // Close the handle to the target process
    CloseHandle(hProcess);

    system("pause");

    return 0;
}
