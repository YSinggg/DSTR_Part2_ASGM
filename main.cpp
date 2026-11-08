#include <iostream>
#include "CampusSystem.hpp"
#include "Task1_ParcelIntake.hpp"
#include "Task2_ServiceRequest.hpp"
#include "Task3_Resource.hpp"
#include "Task4_ActivityHistory.hpp"

using namespace std;

void loadSampleData(CampusSystem& sys) {
    addResource(sys, "C1", "Counter 1", "Counter");
    addResource(sys, "C2", "Counter 2", "Counter");
    addResource(sys, "D1", "Ahmad (rider)", "Delivery");
    addResource(sys, "D2", "Mei Ling (rider)", "Delivery");

    addParcel(sys, "JT100001", "Aisyah Rahman", "TP012345", "Block A Room 3", "J&T", "Normal");
    addParcel(sys, "PL200002", "Daniel Lim", "TP067890", "Block C Room 12", "Pos Laju", "Fragile");
    addParcel(sys, "SP300003", "Dr. Priya Nair", "ST1001", "Level 5 Staff Room", "Shopee Express", "Document");
    addParcel(sys, "NV400004", "Tan Wei Jie", "TP054321", "Block B Room 7", "Ninja Van", "Perishable");
    addParcel(sys, "DH500005", "Hafiz Osman", "TP098765", "Block A Room 9", "DHL", "Normal");
}

void showDashboard(CampusSystem& sys) {
    cout << "  Parcels: " << sys.parcelCount
         << " | Unsorted in Intake: " << sys.intakeQueue.size()
         << " | Waiting Requests: " << sys.requestQueue.size()
         << " | Free Counters: " << sys.counterQueue.size()
         << " | Free Riders: " << sys.deliveryQueue.size() << "\n";

    if (sys.intakeQueue.size() > 0) {
        cout << "  >> Workflow Status: " << sys.intakeQueue.size() 
             << " parcel(s) waiting in Task 1 to be sorted.\n";
    } else if (sys.requestQueue.size() > 0) {
        cout << "  >> Workflow Status: " << sys.requestQueue.size() 
             << " request(s) waiting. Go to Task 3 (or Task 2) to assign a counter/rider.\n";
    } else {
        int assignedCount = 0;
        for (int i = 0; i < sys.requestCount; i++) {
            if (sys.requests[i].status == "Assigned") assignedCount++;
        }
        if (assignedCount > 0) {
            cout << "  >> Workflow Status: " << assignedCount 
                 << " assigned request(s) ready for completion & ID verification in Task 2.\n";
        } else {
            cout << "  >> Workflow Status: All active requests completed. Register more parcels in Task 1.\n";
        }
    }
}

int main() {
    CampusSystem sys;

    cout << "=====================================================\n";
    cout << "     APU SMART CAMPUS PARCEL SYSTEM (DSTR ASGM)\n";
    cout << "=====================================================\n";
    
    int load = readInt("Load sample test data? (1 = Yes, 0 = No): ");
    if (load == 1) {
        loadSampleData(sys);
        cout << "Loaded 5 sample parcels and 4 resources.\n";
    }

    int choice = -1;
    while (choice != 0) {
        cout << "\n--------------------- MAIN MENU ---------------------\n";
        showDashboard(sys);
        cout << "1. Task 1 - Parcel Intake and Sorting (FIFO Queue)\n";
        cout << "2. Task 2 - Service Request Management (Priority Queue)\n";
        cout << "3. Task 3 - Counter & Rider Allocation (Circular Queue)\n";
        cout << "4. Task 4 - Activity Records and Undo (LIFO Stack)\n";
        cout << "0. Exit Program\n";
        choice = readInt("Enter your choice: ");

        switch (choice) {
            case 1:
                task1Menu(sys);
                break;
            case 2:
                task2Menu(sys);
                break;
            case 3:
                task3Menu(sys);
                break;
            case 4:
                task4Menu(sys);
                break;
            case 0:
                cout << "Thank you! Exiting program...\n";
                break;
            default:
                cout << "Invalid selection. Please try again.\n";
        }
    }

    return 0;
}
