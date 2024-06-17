#include <iostream>
#include "ProcessList.h"

using namespace std;

int main() {
    ProcessList processList;

    while (true) {
        int choice;
        cout << "Options:\n1) Show all processes  2) Add new process  3) Delete process 4) Restart Process 5) Exit" << endl;
        cin >> choice;

        try {
            switch (choice) {
                case 1: processList.printList(); break;
                case 2: {
                    string processName;
                    cout << "Enter the name of the process to add (e.g., 'notepad.exe'): ";
                    cin >> processName;
                    if (!processList.addProcess(processName)) {
                        cerr << "Failed to add process." << endl;
                    }
                    break;
                }
                case 3: processList.deleteProcess(); break;
                case 4: processList.restartProcess(); break;
                case 5: return 0;
                default: cout << "Invalid choice." << endl;
            }
        } catch (const exception& e) {
            cerr << e.what() << endl;
        }
    }

    return 0;
}
