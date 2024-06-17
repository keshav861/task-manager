#ifndef PROCESSLIST_H
#define PROCESSLIST_H

#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <string>
#include <exception>

using namespace std;

class Process {
private:
    PROCESSENTRY32 pe32;
    PROCESS_MEMORY_COUNTERS pmc;

public:
    Process() {}
    Process(PROCESSENTRY32 _pe32, PROCESS_MEMORY_COUNTERS _pmc);

    int getPID() const;
    string getImageName() const;
    int getMemUsage() const;
};

class ProcessList {
private:
    vector<Process> listVec;
    void makeList();

public:
    void printList();

    template<typename Compare>
    void sortList(Compare comp);

    void deleteProcess();
    void restartProcess();
    bool addProcess(const string& processName);
};

#endif
