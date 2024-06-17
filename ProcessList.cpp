#include "ProcessList.h"

Process::Process(PROCESSENTRY32 _pe32, PROCESS_MEMORY_COUNTERS _pmc) : pe32(_pe32), pmc(_pmc) {}

int Process::getPID() const {
    return pe32.th32ProcessID;
}

string Process::getImageName() const {
    return string(pe32.szExeFile);
}

int Process::getMemUsage() const {
    return static_cast<int>(pmc.WorkingSetSize / 1024); // Memory usage in KB
}

void ProcessList::makeList() {
    HANDLE hPSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hPSnap == INVALID_HANDLE_VALUE) {
        throw runtime_error("Failed to create process snapshot.");
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hPSnap, &pe32)) {
        do {
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
            if (hProcess) {
                PROCESS_MEMORY_COUNTERS pmc;
                if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
                    listVec.push_back(Process(pe32, pmc));
                }
                CloseHandle(hProcess);
            }
        } while (Process32Next(hPSnap, &pe32));
    }
    CloseHandle(hPSnap);
}

void ProcessList::printList() {
    try {
        makeList();
    } catch (const exception &e) {
        cerr << e.what() << endl;
        return;
    }

    int choice = 0;
    while (true) {
        cout << "Sorting Options:\n1) Name  2) PID  3) Memory Usage  4) Back" << endl;
        cin >> choice;

        if (choice == 4) break; // Go back to main menu
        try {
            switch (choice) {
                case 1: sortList([](const Process& l, const Process& r) { return l.getImageName() < r.getImageName(); }); break;
                case 2: sortList([](const Process& l, const Process& r) { return l.getPID() < r.getPID(); }); break;
                case 3: sortList([](const Process& l, const Process& r) { return l.getMemUsage() < r.getMemUsage(); }); break;
                default: cout << "Invalid choice." << endl; continue;
            }
        } catch (const exception &e) {
            cerr << e.what() << endl;
            return;
        }

        cout << setw(15) << left << "Process ID" << "|" << setw(50) << left << "Image Name" << "| " << setw(20) << right << "Memory Usage (KB)" << endl;
        cout << "-------------------------------------------------------------------------------" << endl;

        for (const auto& process : listVec) {
            cout << setw(15) << left << process.getPID() << "|";
            cout << setw(50) << left << process.getImageName() << "|";
            cout << setw(20) << right << process.getMemUsage() << " KB" << endl;
        }
    }
}

template<typename Compare>
void ProcessList::sortList(Compare comp) {
    sort(listVec.begin(), listVec.end(), comp);
}

void ProcessList::deleteProcess() {
    try {
        makeList();
    } catch (const exception &e) {
        cerr << e.what() << endl;
        return;
    }

    int choice;
    cout << "Delete Options:\n1) Delete by PID  2) Delete by Process Name  3) Back" << endl;
    cin >> choice;

    if (choice == 3) return; // Go back

    DWORD pid = 0;
    string processName;
    if (choice == 1) {
        cout << "Enter the process PID: ";
        cin >> pid;
    } else if (choice == 2) {
        cout << "Enter the process Name: ";
        cin >> processName;
    } else {
        cout << "Invalid choice." << endl;
        return;
    }

    bool success = false;
    for (auto& process : listVec) {
        if ((choice == 1 && process.getPID() == pid) || (choice == 2 && process.getImageName() == processName)) {
            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, process.getPID());
            if (hProcess) {
                if (TerminateProcess(hProcess, 0)) {
                    cout << "Process terminated successfully." << endl;
                    success = true;
                }
                CloseHandle(hProcess);
                break;
            }
        }
    }
    if (!success) cout << "Process not found or unable to terminate." << endl;
}

void ProcessList::restartProcess() {
    try {
        makeList();
    } catch (const exception &e) {
        cerr << e.what() << endl;
        return;
    }

    int choice;
    cout << "Restart Options:\n1) Restart by PID  2) Restart by Process Name  3) Back" << endl;
    cin >> choice;

    if (choice == 3) return; // Go back

    DWORD pid = 0;
    string processName;
    if (choice == 1) {
        cout << "Enter the process PID: ";
        cin >> pid;
    } else if (choice == 2) {
        cout << "Enter the process Name: ";
        cin >> processName;
    } else {
        cout << "Invalid choice." << endl;
        return;
    }

    bool success = false;
    for (auto& process : listVec) {
        if ((choice == 1 && process.getPID() == pid) || (choice == 2 && process.getImageName() == processName)) {
            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, process.getPID());
            if (hProcess) {
                if (TerminateProcess(hProcess, 0)) {
                    cout << "Process terminated successfully." << endl;
                    CloseHandle(hProcess);

                    // Restart the process
                    string command = process.getImageName();
                    STARTUPINFO si = {sizeof(si)};
                    PROCESS_INFORMATION pi;
                    if (CreateProcess(NULL, (LPSTR)command.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
                        cout << "Process restarted successfully." << endl;
                        CloseHandle(pi.hProcess);
                        CloseHandle(pi.hThread);
                        success = true;
                    } else {
                        cerr << "Failed to restart process. Error: " << GetLastError() << endl;
                    }
                    break;
                }
                CloseHandle(hProcess);
            }
        }
    }
    if (!success) cout << "Process not found or unable to terminate." << endl;
}

bool ProcessList::addProcess(const string& processName) {
    STARTUPINFO si = {sizeof(si)};
    PROCESS_INFORMATION pi;

    if (!CreateProcess(NULL, const_cast<char*>(processName.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        cerr << "Failed to start process. Error: " << GetLastError() << endl;
        return false;
    }

    // Close process and thread handles.
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return true;
}
