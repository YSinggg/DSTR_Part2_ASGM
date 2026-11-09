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

bool isValidTrackingNo(string trackingNo, string expectedPrefix = "") {
    if (trackingNo.length() != 8) return false;

    if (trackingNo[0] < 'A' || trackingNo[0] > 'Z') return false;
    if (trackingNo[1] < 'A' || trackingNo[1] > 'Z') return false;

    if (expectedPrefix != "") {
        if (trackingNo.substr(0, 2) != expectedPrefix) return false;
    }

    for (int i = 2; i < 8; i++) {
        if (trackingNo[i] < '0' || trackingNo[i] > '9') return false;
    }
    return true;
}

bool isValidRecipientId(string id) {
    if (id.length() < 6) return false;
    string start = id.substr(0, 2);
    if (start == "TP") {
        if (id.length() != 8) return false;
        for (int i = 2; i < 8; i++) {
            if (id[i] < '0' || id[i] > '9') return false;
        }
        return true;
    } else if (start == "ST") {
        if (id.length() != 6 && id.length() != 7) return false;
        for (int i = 2; i < (int)id.length(); i++) {
            if (id[i] < '0' || id[i] > '9') return false;
        }
        return true;
    }
    return false;
}

bool isValidName(string name) {
    if (name.length() < 2) return false;
    bool hasLetter = false;
    for (int i = 0; i < (int)name.length(); i++) {
        char c = name[i];
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
            hasLetter = true;
        } else if (c != ' ' && c != '\'' && c != '.' && c != '-') {
            return false;
        }
    }
    return hasLetter;
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
        cout << "Error: Parcel storage is full (max " << MAX_PARCELS << ").\n";
        return;
    }

    cout << "Select Courier:\n";
    cout << "  1. J&T Express (JT)\n";
    cout << "  2. Pos Laju (PL)\n";
    cout << "  3. Shopee Express (SP)\n";
    cout << "  4. Ninja Van (NV)\n";
    cout << "  5. DHL Express (DH)\n";
    cout << "  6. Other Courier\n";
    int courierChoice = readInt("Choose courier (1-6): ");

    if (courierChoice < 1 || courierChoice > 6) {
        cout << "Error: Invalid courier choice.\n";
        return;
    }

    string courier = "";
    string expectedPrefix = "";
    if (courierChoice == 1) { courier = "J&T"; expectedPrefix = "JT"; }
    else if (courierChoice == 2) { courier = "Pos Laju"; expectedPrefix = "PL"; }
    else if (courierChoice == 3) { courier = "Shopee Express"; expectedPrefix = "SP"; }
    else if (courierChoice == 4) { courier = "Ninja Van"; expectedPrefix = "NV"; }
    else if (courierChoice == 5) { courier = "DHL"; expectedPrefix = "DH"; }
    else {
        courier = readLine("Enter courier company name: ");
        if (courier == "") {
            cout << "Error: Courier company name cannot be empty!\n";
            return;
        }
        expectedPrefix = "";
    }

    string trackingPrompt = (expectedPrefix != "") ?
        ("Courier tracking number (" + expectedPrefix + " + 6 digits, e.g. " + expectedPrefix + "100001): ") :
        ("Courier tracking number (2 letters + 6 digits, e.g. AB123456): ");
    string trackingNo = toUpper(readLine(trackingPrompt));

    if (!isValidTrackingNo(trackingNo, expectedPrefix)) {
        if (expectedPrefix != "") {
            cout << "Error: Invalid tracking number '" << trackingNo
                 << "'! For " << courier << ", tracking number must start with '"
                 << expectedPrefix << "' followed by exactly 6 digits (e.g. " << expectedPrefix << "100001).\n";
        } else {
            cout << "Error: Invalid tracking number '" << trackingNo
                 << "'! Format must be 2 uppercase letters followed by 6 digits (e.g. AB123456).\n";
        }
        return;
    }

    if (findParcelByTracking(sys, trackingNo) != -1) {
        int dupIdx = findParcelByTracking(sys, trackingNo);
        cout << "Error: Tracking number already exists (" << sys.parcels[dupIdx].parcelId << ")!\n";
        return;
    }

    string name = readLine("Recipient name          : ");
    if (!isValidName(name)) {
        cout << "Error: Invalid recipient name '" << name << "'. Must contain letters.\n";
        return;
    }

    string recipientId = toUpper(readLine("Recipient ID (TP###### / ST####) : "));
    if (!isValidRecipientId(recipientId)) {
        cout << "Error: Invalid recipient ID '" << recipientId
             << "'. Must be TP with 6 digits (e.g. TP070597) or ST with 4-5 digits (e.g. ST1001).\n";
        return;
    }

    string location = readLine("Delivery location       : ");
    if (location == "") {
        cout << "Error: Delivery location cannot be empty!\n";
        return;
    }

    cout << "Parcel types:\n";
    cout << "  1. Normal\n  2. Fragile\n  3. Perishable\n  4. Document\n";
    int typeChoice = readInt("Choose type (1-4): ");
    if (typeChoice < 1 || typeChoice > 4) {
        cout << "Error: Invalid parcel type selected.\n";
        return;
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
    cout << "  1. Search by Parcel ID (e.g. P001)\n";
    cout << "  2. Search by Recipient ID (e.g. TP012345)\n";
    cout << "  3. Search by Tracking Number (e.g. JT100001)\n";
    int choice = readInt("Choose (1-3): ");

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
            if (toUpper(sys.parcels[i].recipientId) == recipientId) {
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
    } else if (choice == 3) {
        string trackingNo = toUpper(readLine("Tracking Number: "));
        int index = findParcelByTracking(sys, trackingNo);
        if (index == -1) {
            cout << "Parcel with tracking number " << trackingNo << " not found.\n";
            return;
        }
        showParcelDetails(sys.parcels[index]);
    } else {
        cout << "Invalid choice!\n";
    }
}

void sortParcel(CampusSystem& sys) {
    cout << "\n--- Sort & Process Incoming Parcel ---\n";
    if (sys.intakeQueue.isEmpty()) {
        cout << "Intake queue is empty. No parcels waiting to be sorted.\n";
        return;
    }

    string nextId = sys.intakeQueue.peek();
    int nextIdx = findParcel(sys, nextId);
    string nextName = (nextIdx != -1) ? sys.parcels[nextIdx].recipientName : "";
    string nextType = (nextIdx != -1) ? sys.parcels[nextIdx].parcelType : "";

    cout << "Waiting in intake queue (" << sys.intakeQueue.size() << "):\n";
    cout << "  [Next FIFO in queue]: " << nextId << " - " << nextName << " (" << nextType << ")\n";
    cout << "  Queue Order        : ";
    ParcelNode* cur = sys.intakeQueue.getFront();
    int countShown = 0;
    while (cur != nullptr && countShown < 8) {
        cout << cur->parcelId;
        if (cur->next != nullptr && countShown < 7) cout << " -> ";
        cur = cur->next;
        countShown++;
    }
    if (sys.intakeQueue.size() > 8) {
        cout << " -> ... (" << (sys.intakeQueue.size() - 8) << " more)";
    }
    cout << "\n\n";

    cout << "Select sorting mode:\n";
    cout << "  1. Sort next parcel in queue (" << nextId << ")\n";
    cout << "  2. Search & sort specific parcel by ID / Tracking No\n";
    cout << "  3. Skip " << nextId << " (move to back of queue)\n";
    cout << "  0. Back to menu\n";
    int mode = readInt("Choose (0-3): ");

    if (mode == 0) return;

    if (mode == 3) {
        if (sys.intakeQueue.size() <= 1) {
            cout << "Notice: Only 1 parcel in intake queue. Cannot rotate.\n";
            return;
        }
        string skipped = sys.intakeQueue.dequeue();
        sys.intakeQueue.enqueue(skipped);
        cout << "[Success] Parcel " << skipped << " moved to the back of intake queue.\n";
        cout << "  -> Next parcel in line is now: " << sys.intakeQueue.peek() << "\n";
        return;
    }

    string targetParcelId = "";
    bool fromHead = false;

    if (mode == 1) {
        targetParcelId = nextId;
        fromHead = true;
    } else if (mode == 2) {
        cout << "\nTip: Enter Parcel ID (e.g. P004) or Tracking Number (e.g. NV400004).\n";
        string query = toUpper(readLine("Enter Parcel ID / Tracking No (or 0 to cancel): "));
        if (query == "0" || query == "") return;

        int idx = findParcel(sys, query);
        if (idx == -1) {
            idx = findParcelByTracking(sys, query);
        }

        if (idx == -1) {
            cout << "Error: Parcel '" << query << "' was not found in system!\n";
            return;
        }

        Parcel& foundP = sys.parcels[idx];
        if (foundP.status != "Registered") {
            cout << "Cannot sort parcel " << foundP.parcelId << "!\n";
            if (foundP.status == "On Hold") {
                cout << "  Reason: Parcel is currently 'On Hold'. To sort it, release it back to the intake queue first in Option 6.\n";
            } else if (foundP.status == "Missing") {
                cout << "  Reason: Parcel is currently marked 'Missing'. Release it first in Option 6 once found.\n";
            } else if (isParcelFinished(foundP.status)) {
                cout << "  Reason: Parcel is already completed (" << foundP.status << ").\n";
            } else {
                cout << "  Reason: Parcel is already sorted (" << foundP.status << ") and has active request " 
                     << foundP.activeRequestId << " in Task 2.\n";
            }
            return;
        }

        targetParcelId = foundP.parcelId;
        fromHead = (targetParcelId == sys.intakeQueue.peek());
    } else {
        cout << "Invalid choice!\n";
        return;
    }

    int index = findParcel(sys, targetParcelId);
    if (index == -1) {
        sys.intakeQueue.removeParcel(targetParcelId);
        cout << "Error: Parcel record missing from system.\n";
        return;
    }
    Parcel& p = sys.parcels[index];
    cout << "\n--- Selected Parcel Details ---\n";
    showParcelDetails(p);

    cout << "\nSelect action for " << p.parcelId << ":\n";
    cout << "  1. Counter collection\n";
    cout << "  2. Campus delivery\n";
    cout << "  3. Special handling (fragile / perishable)\n";
    cout << "  4. Return to sender\n";
    cout << "  5. Place on hold\n";
    if (fromHead && sys.intakeQueue.size() > 1) {
        cout << "  6. Skip this parcel (move to back of queue)\n";
    }
    cout << "  0. Cancel (keep position in queue)\n";
    int actionChoice = readInt("Choose: ");

    if (actionChoice == 0) {
        cout << "Sorting cancelled. Parcel remains in queue.\n";
        return;
    }

    if (fromHead && actionChoice == 6 && sys.intakeQueue.size() > 1) {
        string skipped = sys.intakeQueue.dequeue();
        sys.intakeQueue.enqueue(skipped);
        cout << "[Success] Parcel " << skipped << " moved to the back of the intake queue.\n";
        cout << "  -> Next parcel in line is now: " << sys.intakeQueue.peek() << "\n";
        return;
    }

    if (actionChoice < 1 || actionChoice > 5) {
        cout << "Invalid choice! Parcel remains in queue.\n";
        return;
    }

    if ((p.parcelType == "Fragile" || p.parcelType == "Perishable") && (actionChoice == 1 || actionChoice == 2)) {
        cout << "Notice: Parcel type is '" << p.parcelType
             << "'. Special Handling is strongly recommended.\n";
        int confirm = readInt("Proceed with standard " + string(actionChoice == 1 ? "Collection" : "Delivery") + "? (1 = Yes, 0 = Cancel): ");
        if (confirm != 1) {
            cout << "Sorting cancelled. Parcel remains in queue.\n";
            return;
        }
    }

    if (actionChoice != 5 && sys.requestCount >= MAX_REQUESTS) {
        cout << "Error: Service request storage is full.\n";
        return;
    }

    // Remove from intake queue
    if (fromHead) {
        sys.intakeQueue.dequeue();
    } else {
        sys.intakeQueue.removeParcel(p.parcelId);
    }

    string oldStatus = p.status;

    if (actionChoice == 5) {
        p.status = "On Hold";
        logActivity(sys, p.parcelId, "SORT", "Placed on hold", oldStatus, p.status, "", "", true);
        cout << "\n[Success] Parcel " << p.parcelId << " is now On Hold.\n";
        return;
    }

    string services[4] = {"Collection", "Delivery", "Special Handling", "Return to Sender"};
    string serviceType = services[actionChoice - 1];
    int rIndex = createServiceRequest(sys, p.parcelId, serviceType);
    if (rIndex == -1) {
        cout << "Error: Failed to create service request.\n";
        return;
    }
    string requestId = sys.requests[rIndex].requestId;

    logActivity(sys, p.parcelId, "SORT", "Sorted for " + serviceType + " (" + requestId + ")",
                oldStatus, p.status, requestId, "", true);
    cout << "\n[Success] Parcel " << p.parcelId << " sorted for " << serviceType << "!\n";
    cout << "  -> Service Request " << requestId << " automatically created and added to Task 2 waiting queue.\n";
    cout << "  -> Next Step: Go to Task 3 (or Task 2 Option 4) to assign a counter/rider.\n";
}

void sortNextParcel(CampusSystem& sys) {
    sortParcel(sys);
}

void updateParcel(CampusSystem& sys) {
    cout << "\n--- Update or Remove Parcel ---\n";
    if (sys.parcelCount == 0) {
        cout << "No parcels registered in system.\n";
        return;
    }
    cout << "Parcels in system: ";
    for (int i = 0; i < sys.parcelCount; i++) {
        cout << sys.parcels[i].parcelId << " (" << sys.parcels[i].status << ")";
        if (i < sys.parcelCount - 1) cout << ", ";
    }
    cout << "\n";
    string parcelId = toUpper(readLine("Enter Parcel ID (or 0 to cancel): "));
    if (parcelId == "0" || parcelId == "") return;
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
        string recipientId = toUpper(readLine("New recipient ID (TP###### / ST####): "));
        string location = readLine("New location      : ");

        if (name != "" && !isValidName(name)) {
            cout << "Error: Invalid recipient name '" << name << "'. Must contain letters.\n";
            return;
        }
        if (recipientId != "" && !isValidRecipientId(recipientId)) {
            cout << "Error: Invalid recipient ID format. Must be TP with 6 digits or ST with 4-5 digits.\n";
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
        cout << "  5. Sort parcel (Next in queue / Search by ID)\n";
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
