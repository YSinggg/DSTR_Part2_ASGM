#ifndef TASK1_PARCEL_INTAKE_HPP
#define TASK1_PARCEL_INTAKE_HPP

#include "CampusSystem.hpp"

void printParcelHeader() {
    cout << "  " << left << setw(8) << "ID" << setw(14) << "Tracking"
         << setw(18) << "Recipient" << setw(10) << "Recip ID"
         << setw(14) << "Type" << "Status" << "\n";
    cout << "  ----------------------------------------------------------------------\n";
}

void printParcelRow(Parcel& p) {
    cout << "  " << left << setw(8) << p.parcelId
         << setw(14) << p.trackingNo
         << setw(18) << p.recipientName
         << setw(10) << p.recipientId
         << setw(14) << p.parcelType
         << p.status << "\n";
}

void showParcelDetails(Parcel& p) {
    cout << "  Parcel ID      : " << p.parcelId << "\n";
    cout << "  Tracking no.   : " << p.trackingNo << "\n";
    cout << "  Recipient      : " << p.recipientName << " (" << p.recipientId << ")\n";
    cout << "  Location       : " << p.location << "\n";
    cout << "  Courier        : " << p.courier << "\n";
    cout << "  Parcel type    : " << p.parcelType << "\n";
    cout << "  Arrived        : " << p.arrivalTime << "\n";
    cout << "  Status         : " << p.status << "\n";
    if (p.activeRequestId != "") {
        cout << "  Active request : " << p.activeRequestId << "\n";
    } else {
        cout << "  Active request : None\n";
    }
}

bool isValidRecipientId(string id) {
    if (id.length() < 5) return false;
    string start = id.substr(0, 2);
    if (start != "TP" && start != "ST") return false;
    for (int i = 2; i < (int)id.length(); i++) {
        if (id[i] < '0' || id[i] > '9') return false;
    }
    return true;
}

string addParcel(CampusSystem& sys, string trackingNo, string name, string recipientId,
                 string location, string courier, string parcelType) {
    Parcel p;
    p.parcelId = makeId("P", sys.nextParcelNo++);
    p.trackingNo = trackingNo;
    p.recipientName = name;
    p.recipientId = recipientId;
    p.location = location;
    p.courier = courier;
    p.parcelType = parcelType;
    p.arrivalTime = currentTime();
    p.status = "Registered";
    p.activeRequestId = "";

    sys.parcels[sys.parcelCount++] = p;
    sys.intakeQueue.enqueue(p.parcelId);

    logActivity(sys, p.parcelId, "REGISTER", "Parcel registered (" + trackingNo + ")",
                "-", "Registered", "", "", false);
    return p.parcelId;
}

void registerParcel(CampusSystem& sys) {
    cout << "\n--- Register New Parcel ---\n";
    if (sys.parcelCount >= MAX_PARCELS) {
        cout << "Error: Parcel storage is full.\n";
        return;
    }

    string trackingNo = toUpper(readLine("Courier tracking number : "));
    string name = readLine("Recipient name          : ");
    string recipientId = toUpper(readLine("Recipient ID (TP/ST)    : "));
    string location = readLine("Delivery location       : ");
    string courier = readLine("Courier company         : ");
    cout << "Parcel types:\n";
    cout << "  1. Normal\n  2. Fragile\n  3. Perishable\n  4. Document\n";
    int typeChoice = readInt("Choose type (1-4): ");

    if (trackingNo == "" || name == "" || recipientId == "" || location == "" || courier == "") {
        cout << "Error: All fields are required!\n";
        return;
    }
    if (!isValidRecipientId(recipientId)) {
        cout << "Error: Invalid recipient ID '" << recipientId << "'. Must start with TP or ST.\n";
        return;
    }
    if (typeChoice < 1 || typeChoice > 4) {
        cout << "Error: Invalid parcel type selected.\n";
        return;
    }
    for (int i = 0; i < sys.parcelCount; i++) {
        if (sys.parcels[i].trackingNo == trackingNo) {
            cout << "Error: Tracking number already exists (" << sys.parcels[i].parcelId << ")!\n";
            return;
        }
    }

    string types[4] = {"Normal", "Fragile", "Perishable", "Document"};
    string parcelId = addParcel(sys, trackingNo, name, recipientId, location, courier, types[typeChoice - 1]);
    cout << "Success: Parcel registered as " << parcelId << " and added to intake queue.\n";
    cout << "Parcels waiting to be sorted: " << sys.intakeQueue.size() << "\n";
}

void displayAllParcels(CampusSystem& sys) {
    cout << "\n--- All Parcel Records ---\n";
    if (sys.parcelCount == 0) {
        cout << "No parcels registered yet.\n";
        return;
    }
    printParcelHeader();
    for (int i = 0; i < sys.parcelCount; i++) {
        printParcelRow(sys.parcels[i]);
    }
    cout << "Total: " << sys.parcelCount << " parcel(s)\n";
}

void viewIntakeQueue(CampusSystem& sys) {
    cout << "\n--- Intake Queue (FIFO) ---\n";
    if (sys.intakeQueue.isEmpty()) {
        cout << "Intake queue is empty.\n";
        return;
    }
    printParcelHeader();
    ParcelNode* current = sys.intakeQueue.getFront();
    while (current != nullptr) {
        int index = findParcel(sys, current->parcelId);
        if (index != -1) {
            printParcelRow(sys.parcels[index]);
        }
        current = current->next;
    }
    cout << sys.intakeQueue.size() << " parcel(s) waiting to be sorted.\n";
}

void searchParcel(CampusSystem& sys) {
    cout << "\n--- Search Parcel ---\n";
    cout << "  1. Search by Parcel ID\n";
    cout << "  2. Search by Recipient ID\n";
    int choice = readInt("Choose: ");

    if (choice == 1) {
        string parcelId = toUpper(readLine("Parcel ID: "));
        int index = findParcel(sys, parcelId);
        if (index == -1) {
            cout << "Parcel " << parcelId << " not found.\n";
            return;
        }
        showParcelDetails(sys.parcels[index]);
    } else if (choice == 2) {
        string recipientId = toUpper(readLine("Recipient ID: "));
        int found = 0;
        for (int i = 0; i < sys.parcelCount; i++) {
            if (sys.parcels[i].recipientId == recipientId) {
                if (found == 0) {
                    printParcelHeader();
                }
                printParcelRow(sys.parcels[i]);
                found++;
            }
        }
        if (found == 0) {
            cout << "No parcels found for recipient " << recipientId << ".\n";
        }
    } else {
        cout << "Invalid choice!\n";
    }
}

void sortNextParcel(CampusSystem& sys) {
    cout << "\n--- Sort Next Parcel ---\n";
    if (sys.intakeQueue.isEmpty()) {
        cout << "Intake queue is empty. No parcel to sort.\n";
        return;
    }

    string parcelId = sys.intakeQueue.peek();
    int index = findParcel(sys, parcelId);
    Parcel& p = sys.parcels[index];
    showParcelDetails(p);

    cout << "\nSelect action:\n";
    cout << "  1. Counter collection\n";
    cout << "  2. Campus delivery\n";
    cout << "  3. Special handling (fragile / perishable)\n";
    cout << "  4. Return to sender\n";
    cout << "  5. Place on hold\n";
    cout << "  0. Skip for now\n";
    int choice = readInt("Choose (0-5): ");

    if (choice == 0) {
        cout << "Parcel remains in queue.\n";
        return;
    }
    if (choice < 1 || choice > 5) {
        cout << "Invalid choice! Parcel remains in queue.\n";
        return;
    }
    if (choice != 5 && sys.requestCount >= MAX_REQUESTS) {
        cout << "Error: Service request storage is full.\n";
        return;
    }

    sys.intakeQueue.dequeue();
    string oldStatus = p.status;

    if (choice == 5) {
        p.status = "On Hold";
        logActivity(sys, parcelId, "SORT", "Placed on hold", oldStatus, p.status, "", "", true);
        cout << parcelId << " is now On Hold.\n";
        return;
    }

    string services[4] = {"Collection", "Delivery", "Special Handling", "Return to Sender"};
    string serviceType = services[choice - 1];
    int rIndex = createServiceRequest(sys, parcelId, serviceType);
    string requestId = sys.requests[rIndex].requestId;

    logActivity(sys, parcelId, "SORT", "Sorted for " + serviceType + " (" + requestId + ")",
                oldStatus, p.status, requestId, "", true);
    cout << "Sorted " << parcelId << " for " << serviceType << ". Request " << requestId << " created.\n";
}

void updateParcel(CampusSystem& sys) {
    cout << "\n--- Update or Remove Parcel ---\n";
    string parcelId = toUpper(readLine("Parcel ID: "));
    int index = findParcel(sys, parcelId);
    if (index == -1) {
        cout << "Parcel " << parcelId << " does not exist.\n";
        return;
    }
    Parcel& p = sys.parcels[index];
    showParcelDetails(p);

    if (isParcelFinished(p.status)) {
        cout << "Parcel is already " << p.status << " and cannot be modified.\n";
        return;
    }

    cout << "\n  1. Update recipient info\n";
    cout << "  2. Report as missing\n";
    cout << "  3. Release from hold / found (re-queue to intake)\n";
    cout << "  4. Cancel parcel\n";
    cout << "  0. Back\n";
    int choice = readInt("Choice: ");
    string oldStatus = p.status;

    if (choice == 1) {
        cout << "Press Enter to keep current value.\n";
        string name = readLine("New recipient name: ");
        string recipientId = toUpper(readLine("New recipient ID  : "));
        string location = readLine("New location      : ");
        if (recipientId != "" && !isValidRecipientId(recipientId)) {
            cout << "Error: Invalid recipient ID format.\n";
            return;
        }
        if (name != "") p.recipientName = name;
        if (recipientId != "") p.recipientId = recipientId;
        if (location != "") p.location = location;
        logActivity(sys, parcelId, "UPDATE", "Recipient info updated", oldStatus, p.status, "", "", false);
        cout << "Recipient info updated successfully.\n";
    }
    else if (choice == 2 || choice == 4) {
        if (p.activeRequestId != "") {
            cout << "Error: Parcel has active request " << p.activeRequestId << ". Cancel it first in Task 2.\n";
            return;
        }
        if (choice == 2 && p.status == "Missing") {
            cout << "Parcel is already marked Missing.\n";
            return;
        }
        if (p.status == "Registered") {
            sys.intakeQueue.removeParcel(parcelId);
        }

        if (choice == 2) {
            p.status = "Missing";
            logActivity(sys, parcelId, "UPDATE", "Reported missing", oldStatus, p.status, "", "", false);
            cout << parcelId << " is now marked as Missing.\n";
        } else {
            p.status = "Cancelled";
            logActivity(sys, parcelId, "UPDATE", "Parcel cancelled", oldStatus, p.status, "", "", false);
            cout << parcelId << " cancelled.\n";
        }
    }
    else if (choice == 3) {
        if (p.status != "Missing" && p.status != "On Hold") {
            cout << "Only Missing or On Hold parcels can be released.\n";
            return;
        }
        p.status = "Registered";
        sys.intakeQueue.enqueue(parcelId);
        logActivity(sys, parcelId, "UPDATE", "Released back to intake queue", oldStatus, p.status, "", "", false);
        cout << parcelId << " is now back in intake queue.\n";
    }
    else if (choice != 0) {
        cout << "Invalid choice!\n";
    }
}

void task1Menu(CampusSystem& sys) {
    int choice = -1;
    while (choice != 0) {
        cout << "\n========== TASK 1: PARCEL INTAKE & SORTING ==========\n";
        cout << "  1. Register new parcel\n";
        cout << "  2. Display all parcels\n";
        cout << "  3. View intake queue\n";
        cout << "  4. Search parcel\n";
        cout << "  5. Sort next parcel\n";
        cout << "  6. Update / remove parcel\n";
        cout << "  0. Back to main menu\n";
        choice = readInt("Choose: ");

        switch (choice) {
            case 1: registerParcel(sys); break;
            case 2: displayAllParcels(sys); break;
            case 3: viewIntakeQueue(sys); break;
            case 4: searchParcel(sys); break;
            case 5: sortNextParcel(sys); break;
            case 6: updateParcel(sys); break;
            case 0: break;
            default: cout << "Invalid choice!\n";
        }
    }
}

#endif
