#ifndef TASK4_ACTIVITY_HISTORY_HPP
#define TASK4_ACTIVITY_HISTORY_HPP

#include "CampusSystem.hpp"

const int MAX_HISTORY_VIEW = 100;

void printActivity(Activity& a) {
    cout << "  #" << left << setw(4) << a.activityNo << a.time << "  "
         << setw(8) << a.parcelId << setw(10) << a.type
         << a.oldStatus << " -> " << a.newStatus << "\n";
    cout << "        " << a.description;
    if (a.reversed) {
        cout << "  [REVERSED]";
    }
    cout << "\n";
}

void showRecentActivities(CampusSystem& sys) {
    cout << "\n--- Recent Activities (Newest First) ---\n";
    if (sys.history.isEmpty()) {
        cout << "No activity records found.\n";
        return;
    }
    int count = readInt("How many activities to show? ");
    if (count <= 0) {
        cout << "Please enter a positive number.\n";
        return;
    }

    ActivityNode* current = sys.history.getTop();
    int shown = 0;
    while (current != nullptr && shown < count) {
        printActivity(current->data);
        current = current->next;
        shown++;
    }
    cout << "Showing " << shown << " of " << sys.history.size() << " activities.\n";
}

void reviewParcelHistory(CampusSystem& sys) {
    cout << "\n--- Parcel Activity History ---\n";
    if (sys.parcelCount == 0) {
        cout << "No parcels registered in system.\n";
        return;
    }
    cout << "Parcels in system: ";
    for (int i = 0; i < sys.parcelCount; i++) {
        cout << sys.parcels[i].parcelId;
        if (i < sys.parcelCount - 1) cout << ", ";
    }
    cout << "\n";
    string parcelId = toUpper(readLine("Enter Parcel ID to review (or 0 to cancel): "));
    if (parcelId == "0" || parcelId == "") return;
    int pIndex = findParcel(sys, parcelId);
    if (pIndex == -1) {
        cout << "Error: Parcel ID " << parcelId << " not found.\n";
        return;
    }
    Parcel& p = sys.parcels[pIndex];

    Activity* found[MAX_HISTORY_VIEW];
    int count = 0;
    ActivityNode* current = sys.history.getTop();
    while (current != nullptr && count < MAX_HISTORY_VIEW) {
        if (current->data.parcelId == parcelId) {
            found[count++] = &current->data;
        }
        current = current->next;
    }

    if (count == 0) {
        cout << "No history found for parcel " << parcelId << ".\n";
        return;
    }

    cout << "\nSelect display order:\n";
    cout << "  1. Newest first (LIFO - Stack Order)\n";
    cout << "  2. Oldest first (Chronological Timeline)\n";
    int order = readInt("Choose order (1-2): ");

    if (order == 2) {
        cout << "\n--- Activities in Chronological Order (Oldest First) ---\n";
        for (int i = count - 1; i >= 0; i--) {
            printActivity(*found[i]);
        }
    } else {
        cout << "\n--- Activities in LIFO Order (Newest First - Top of Stack) ---\n";
        for (int i = 0; i < count; i++) {
            printActivity(*found[i]);
        }
    }

    Activity* latest = found[0];
    cout << "\nCurrent Status  : " << p.status << "\n";
    cout << "Previous Status : " << (latest->oldStatus == "-" ? "None" : latest->oldStatus) << "\n";
    cout << "Latest Action   : #" << latest->activityNo << " " << latest->type
         << " - " << latest->description << "\n";
    if (p.activeRequestId != "") {
        int rIdx = findRequest(sys, p.activeRequestId);
        if (rIdx != -1) {
            ServiceRequest& r = sys.requests[rIdx];
            cout << "Active Request  : " << r.requestId << " (" << r.serviceType << ", " << r.status << ")";
            if (r.resourceId != "") {
                cout << " handled by " << r.resourceId;
            }
            cout << "\n";
        }
    } else {
        cout << "Active Request  : None\n";
    }
}

bool undoSort(CampusSystem& sys, Activity* a, Parcel& p) {
    if (a->requestId != "") {
        int rIndex = findRequest(sys, a->requestId);
        if (rIndex == -1) return false;
        ServiceRequest& r = sys.requests[rIndex];
        if (r.status != "Waiting") {
            return false;
        }
        sys.requestQueue.removeRequest(r.requestId);
        r.status = "Cancelled";
        p.activeRequestId = "";
    }
    p.status = "Registered";
    sys.intakeQueue.enqueue(p.parcelId);
    return true;
}

bool undoAssign(CampusSystem& sys, Activity* a) {
    int rIndex = findRequest(sys, a->requestId);
    if (rIndex == -1 || sys.requests[rIndex].status != "Assigned") {
        return false;
    }
    releaseResource(sys, a->resourceId);
    returnRequestToWaiting(sys, rIndex);
    return true;
}

void undoActivity(CampusSystem& sys) {
    cout << "\n--- Undo Recent Activity ---\n";
    if (sys.history.isEmpty()) {
        cout << "No activities to undo.\n";
        return;
    }

    cout << "Recent activities:\n";
    ActivityNode* current = sys.history.getTop();
    int canUndoCount = 0;
    for (int i = 0; i < 6 && current != nullptr; i++) {
        Activity& a = current->data;
        cout << "  #" << left << setw(4) << a.activityNo << a.time << "  "
             << setw(8) << a.parcelId << setw(10) << a.type
             << a.oldStatus << " -> " << a.newStatus;
        
        if (a.reversed) {
            cout << "  [ALREADY UNDONE]";
        } else if (!a.canUndo) {
            cout << "  (Cannot undo " << a.type << ")";
        } else if (sys.history.latestActivityFor(a.parcelId) != a.activityNo) {
            cout << "  (Parcel modified since then)";
        } else {
            cout << "  [CAN UNDO]";
            canUndoCount++;
        }
        cout << "\n";
        current = current->next;
    }

    if (canUndoCount == 0) {
        cout << "\nNotice: None of the recent activities can be undone right now.\n";
        return;
    }

    int no = readInt("Enter activity number to undo (or 0 to cancel): ");
    if (no == 0) return;

    Activity* a = sys.history.findActivity(no);
    if (a == nullptr) {
        cout << "Error: Activity #" << no << " does not exist.\n";
        return;
    }
    if (a->reversed) {
        cout << "Error: Activity #" << no << " was already reversed.\n";
        return;
    }
    if (!a->canUndo) {
        cout << "Error: Activity #" << no << " (" << a->type << ") cannot be undone.\n";
        return;
    }

    int pIndex = findParcel(sys, a->parcelId);
    if (pIndex == -1) {
        cout << "Error: Parcel not found.\n";
        return;
    }
    Parcel& p = sys.parcels[pIndex];

    if (sys.history.latestActivityFor(p.parcelId) != no || p.status != a->newStatus) {
        cout << "Error: Cannot undo #" << no << ". Parcel status has changed since this action.\n";
        return;
    }

    string statusBefore = p.status;
    bool done = false;
    if (a->type == "SORT") {
        done = undoSort(sys, a, p);
    } else if (a->type == "ASSIGN") {
        done = undoAssign(sys, a);
    }

    if (!done) {
        cout << "Undo failed: Associated request is no longer in valid state.\n";
        return;
    }

    a->reversed = true;
    logActivity(sys, p.parcelId, "UNDO", "Reversed activity #" + to_string(no) + " (" + a->type + ")",
                statusBefore, p.status, a->requestId, a->resourceId, false);
    cout << "Activity #" << no << " reversed successfully. Parcel " << p.parcelId
         << " status restored to: " << p.status << "\n";
}

void task4Menu(CampusSystem& sys) {
    int choice = -1;
    while (choice != 0) {
        cout << "\n========== TASK 4: ACTIVITY LOG & UNDO ==========\n";
        cout << "  1. View recent activities\n";
        cout << "  2. Review parcel activity history\n";
        cout << "  3. Undo an activity\n";
        cout << "  0. Back to main menu\n";
        choice = readInt("Choose: ");

        switch (choice) {
            case 1: showRecentActivities(sys); break;
            case 2: reviewParcelHistory(sys); break;
            case 3: undoActivity(sys); break;
            case 0: break;
            default: cout << "Invalid choice!\n";
        }
    }
}

#endif
