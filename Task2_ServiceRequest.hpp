#ifndef TASK2_SERVICE_REQUEST_HPP
#define TASK2_SERVICE_REQUEST_HPP

#include "CampusSystem.hpp"

int createServiceRequest(CampusSystem& sys, string parcelId, string serviceType) {
    if (sys.requestCount >= MAX_REQUESTS) {
        return -1;
    }
    ServiceRequest r;
    r.requestId = makeId("R", sys.nextRequestNo);
    r.orderNo = sys.nextRequestNo++;
    r.parcelId = parcelId;
    r.serviceType = serviceType;
    r.priority = priorityFor(serviceType);
    r.requestTime = currentTime();
    r.status = "Waiting";
    r.resourceId = "";

    sys.requests[sys.requestCount] = r;
    sys.requestQueue.enqueue(r.requestId, r.priority, r.orderNo);

    int pIndex = findParcel(sys, parcelId);
    sys.parcels[pIndex].activeRequestId = r.requestId;
    sys.parcels[pIndex].status = waitingStatusFor(serviceType);
    return sys.requestCount++;
}

void returnRequestToWaiting(CampusSystem& sys, int rIndex) {
    ServiceRequest& r = sys.requests[rIndex];
    r.status = "Waiting";
    r.resourceId = "";
    sys.requestQueue.enqueue(r.requestId, r.priority, r.orderNo);

    int pIndex = findParcel(sys, r.parcelId);
    sys.parcels[pIndex].status = waitingStatusFor(r.serviceType);
}

void printRequestHeader() {
    cout << "  " << left << setw(8) << "ID" << setw(10) << "Parcel" << setw(20) << "Service"
         << setw(6) << "Pri" << setw(14) << "Status" << setw(10) << "Resource" << "Created" << "\n";
    cout << "  -------------------------------------------------------------------------------\n";
}

void printRequestRow(ServiceRequest& r) {
    cout << "  " << left << setw(8) << r.requestId << setw(10) << r.parcelId
         << setw(20) << r.serviceType << setw(6) << r.priority << setw(14) << r.status
         << setw(10) << (r.resourceId == "" ? "-" : r.resourceId) << r.requestTime << "\n";
}

void createRequestMenu(CampusSystem& sys) {
    cout << "\n--- Create Service Request ---\n";
    if (sys.requestCount >= MAX_REQUESTS) {
        cout << "Error: Request list is full.\n";
        return;
    }
    string parcelId = toUpper(readLine("Parcel ID: "));
    int pIndex = findParcel(sys, parcelId);
    if (pIndex == -1) {
        cout << "Error: Parcel ID " << parcelId << " not found.\n";
        return;
    }
    Parcel& p = sys.parcels[pIndex];

    if (isParcelFinished(p.status)) {
        cout << "Error: Parcel is already " << p.status << ".\n";
        return;
    }
    if (p.activeRequestId != "") {
        cout << "Error: Parcel already has active request " << p.activeRequestId << ".\n";
        return;
    }
    if (p.status == "Registered") {
        cout << "Error: Parcel not sorted yet. Please sort it in Task 1 first.\n";
        return;
    }
    if (p.status == "Missing" || p.status == "On Hold") {
        cout << "Error: Parcel is " << p.status << ". Release it first.\n";
        return;
    }

    cout << "Service types:\n";
    cout << "  1. Collection\n  2. Delivery\n  3. Special Handling\n  4. Re-delivery\n  5. Return to Sender\n";
    int choice = readInt("Choose type (1-5): ");
    if (choice < 1 || choice > 5) {
        cout << "Invalid choice!\n";
        return;
    }
    string services[5] = {"Collection", "Delivery", "Special Handling", "Re-delivery", "Return to Sender"};

    string oldStatus = p.status;
    int rIndex = createServiceRequest(sys, parcelId, services[choice - 1]);
    ServiceRequest& r = sys.requests[rIndex];
    logActivity(sys, parcelId, "REQUEST", r.serviceType + " request " + r.requestId + " created",
                oldStatus, p.status, r.requestId, "", false);
    cout << "Request " << r.requestId << " created with priority " << r.priority << ".\n";
}

void viewWaitingRequests(CampusSystem& sys) {
    cout << "\n--- Waiting Requests (Priority Order) ---\n";
    if (sys.requestQueue.isEmpty()) {
        cout << "No waiting requests.\n";
        return;
    }
    printRequestHeader();
    RequestNode* current = sys.requestQueue.getFront();
    while (current != nullptr) {
        int index = findRequest(sys, current->requestId);
        if (index != -1) {
            printRequestRow(sys.requests[index]);
        }
        current = current->next;
    }
    cout << "Next to handle: " << sys.requestQueue.peek() << "\n";
}

void viewAllRequests(CampusSystem& sys) {
    cout << "\n--- All Service Requests ---\n";
    if (sys.requestCount == 0) {
        cout << "No requests recorded.\n";
        return;
    }
    printRequestHeader();
    for (int i = 0; i < sys.requestCount; i++) {
        printRequestRow(sys.requests[i]);
    }
}

void completeRequest(CampusSystem& sys) {
    cout << "\n--- Complete Request ---\n";
    string requestId = toUpper(readLine("Request ID: "));
    int rIndex = findRequest(sys, requestId);
    if (rIndex == -1) {
        cout << "Error: Request ID not found.\n";
        return;
    }
    ServiceRequest& r = sys.requests[rIndex];

    if (r.status == "Completed" || r.status == "Failed") {
        cout << "Error: Request already finished (" << r.status << ").\n";
        return;
    }
    if (r.status == "Cancelled") {
        cout << "Error: Request is cancelled.\n";
        return;
    }
    if (r.status == "Waiting") {
        cout << "Error: Request is still waiting. Assign a resource first.\n";
        return;
    }

    int pIndex = findParcel(sys, r.parcelId);
    Parcel& p = sys.parcels[pIndex];
    bool failed = false;

    if (resourceTypeFor(r.serviceType) == "Delivery") {
        cout << "  1. Handed to recipient\n";
        cout << "  2. Recipient unavailable (failed)\n";
        int result = readInt("Result: ");
        if (result == 2) {
            failed = true;
        } else if (result != 1) {
            cout << "Invalid choice!\n";
            return;
        }
    }

    if (!failed && r.serviceType != "Return to Sender") {
        string shownId = toUpper(readLine("Recipient verification ID: "));
        if (shownId != p.recipientId) {
            cout << "Error: Recipient ID does not match! Parcel not delivered.\n";
            return;
        }
    }

    string oldStatus = p.status;
    string resId = r.resourceId;
    if (failed) {
        r.status = "Failed";
        p.status = "Delivery Failed";
    } else {
        r.status = "Completed";
        if (r.serviceType == "Collection") {
            p.status = "Collected";
        } else if (r.serviceType == "Return to Sender") {
            p.status = "Returned to Sender";
        } else {
            p.status = "Delivered";
        }
    }
    p.activeRequestId = "";
    releaseResource(sys, resId);

    string result = failed ? "failed" : "completed";
    logActivity(sys, p.parcelId, "COMPLETE",
                r.serviceType + " request " + requestId + " " + result + " by " + resId,
                oldStatus, p.status, requestId, resId, false);
    cout << "Request " << requestId << " " << result << ". Parcel " << p.parcelId
         << " is now: " << p.status << "\n";
}

void cancelRequest(CampusSystem& sys) {
    cout << "\n--- Cancel Service Request ---\n";
    string requestId = toUpper(readLine("Request ID: "));
    int rIndex = findRequest(sys, requestId);
    if (rIndex == -1) {
        cout << "Error: Request not found.\n";
        return;
    }
    ServiceRequest& r = sys.requests[rIndex];

    if (r.status == "Cancelled") {
        cout << "Request is already cancelled.\n";
        return;
    }
    if (r.status == "Completed" || r.status == "Failed") {
        cout << "Cannot cancel: Request is already finished.\n";
        return;
    }

    if (r.status == "Waiting") {
        sys.requestQueue.removeRequest(requestId);
    } else {
        releaseResource(sys, r.resourceId);
    }
    r.status = "Cancelled";

    int pIndex = findParcel(sys, r.parcelId);
    Parcel& p = sys.parcels[pIndex];
    string oldStatus = p.status;
    p.status = "Request Cancelled";
    p.activeRequestId = "";

    logActivity(sys, p.parcelId, "CANCEL", "Request " + requestId + " cancelled",
                oldStatus, p.status, requestId, r.resourceId, false);
    cout << "Request " << requestId << " cancelled.\n";
}

void task2Menu(CampusSystem& sys) {
    int choice = -1;
    while (choice != 0) {
        cout << "\n========== TASK 2: SERVICE REQUEST MANAGEMENT ==========\n";
        cout << "  1. Create service request\n";
        cout << "  2. View waiting requests (priority order)\n";
        cout << "  3. View all requests\n";
        cout << "  4. Process next request\n";
        cout << "  5. Complete request\n";
        cout << "  6. Cancel request\n";
        cout << "  0. Back to main menu\n";
        choice = readInt("Choose: ");

        switch (choice) {
            case 1: createRequestMenu(sys); break;
            case 2: viewWaitingRequests(sys); break;
            case 3: viewAllRequests(sys); break;
            case 4: assignNextRequest(sys); break;
            case 5: completeRequest(sys); break;
            case 6: cancelRequest(sys); break;
            case 0: break;
            default: cout << "Invalid choice!\n";
        }
    }
}

#endif
