#ifndef TASK3_RESOURCE_HPP
#define TASK3_RESOURCE_HPP

#include "CampusSystem.hpp"

ResourceCircularQueue& queueFor(CampusSystem& sys, string type) {
    if (type == "Counter") {
        return sys.counterQueue;
    }
    return sys.deliveryQueue;
}

int countResourcesOfType(CampusSystem& sys, string type) {
    int total = 0;
    for (int i = 0; i < sys.resourceCount; i++) {
        if (sys.resources[i].type == type) {
            total++;
        }
    }
    return total;
}

// Format check for Resource IDs (e.g. C1 for Counter, D1 for Delivery)
bool isValidResourceId(string id, string type) {
    if (id.length() < 2 || id.length() > 4) return false;
    char expected = (type == "Counter") ? 'C' : 'D';
    if (id[0] != expected) return false;
    for (int i = 1; i < (int)id.length(); i++) {
        if (id[i] < '0' || id[i] > '9') return false;
    }
    return true;
}

void addResource(CampusSystem& sys, string id, string name, string type) {
    Resource res;
    res.resourceId = id;
    res.name = name;
    res.type = type;
    res.status = "Available";
    res.requestId = "";
    sys.resources[sys.resourceCount++] = res;
    queueFor(sys, type).enqueue(id);
}

void releaseResource(CampusSystem& sys, string resourceId) {
    int resIndex = findResource(sys, resourceId);
    if (resIndex == -1) {
        cout << "Error: Resource " << resourceId << " not found.\n";
        return;
    }
    Resource& res = sys.resources[resIndex];
    if (res.status != "Occupied") {
        return;
    }
    res.status = "Available";
    res.requestId = "";
    if (!queueFor(sys, res.type).enqueue(resourceId)) {
        cout << "Warning: " << res.type << " queue capacity reached.\n";
    }
}

void doAssignment(CampusSystem& sys, int rIndex, int resIndex) {
    ServiceRequest& r = sys.requests[rIndex];
    Resource& res = sys.resources[resIndex];
    ResourceCircularQueue& freeQueue = queueFor(sys, res.type);

    sys.requestQueue.removeRequest(r.requestId);

    if (freeQueue.peek() == res.resourceId) {
        freeQueue.dequeue();
    } else {
        freeQueue.removeResource(res.resourceId);
    }

    res.status = "Occupied";
    res.requestId = r.requestId;
    r.status = "Assigned";
    r.resourceId = res.resourceId;

    int pIndex = findParcel(sys, r.parcelId);
    if (pIndex != -1) {
        Parcel& p = sys.parcels[pIndex];
        string oldStatus = p.status;
        p.status = "Assigned to " + res.resourceId;

        logActivity(sys, p.parcelId, "ASSIGN",
                    r.serviceType + " request " + r.requestId + " assigned to " + res.resourceId,
                    oldStatus, p.status, r.requestId, res.resourceId, true);
    }
    cout << "Assigned " << r.requestId << " to " << res.resourceId << " (" << res.name << ").\n";
}

void registerResource(CampusSystem& sys) {
    cout << "\n--- Register Resource ---\n";
    if (sys.resourceCount >= MAX_RESOURCES) {
        cout << "Error: Max limit of " << MAX_RESOURCES << " resources reached.\n";
        return;
    }

    cout << "Resource Type:\n  1. Counter (e.g. C1, C2)\n  2. Delivery Staff (e.g. D1, D2)\n";
    int typeChoice = readInt("Choose (1-2): ");
    if (typeChoice != 1 && typeChoice != 2) {
        cout << "Error: Invalid choice.\n";
        return;
    }

    string type = (typeChoice == 1) ? "Counter" : "Delivery";
    char prefixChar = (type == "Counter") ? 'C' : 'D';
    string exampleId = (type == "Counter") ? "C3" : "D3";

    string id = toUpper(readLine("Resource ID (" + string(1, prefixChar) + " + number, e.g. " + exampleId + "): "));
    string name = readLine("Resource Name: ");

    if (id == "" || name == "") {
        cout << "Error: ID and Name cannot be empty.\n";
        return;
    }
    if (!isValidResourceId(id, type)) {
        cout << "Error: Invalid resource ID '" << id << "'! " << type
             << " ID must start with '" << prefixChar << "' followed by digits (e.g. " << exampleId << ").\n";
        return;
    }
    if (findResource(sys, id) != -1) {
        cout << "Error: Resource ID already exists.\n";
        return;
    }
    if (queueFor(sys, type).isFull()) {
        cout << "Error: " << type << " queue is full.\n";
        return;
    }
    addResource(sys, id, name, type);
    cout << "Registered " << id << " as available " << type << ".\n";
}

void printFreeQueue(string label, ResourceCircularQueue& q) {
    cout << "  " << label << ": ";
    if (q.isEmpty()) {
        cout << "none\n";
        return;
    }
    for (int i = 0; i < q.size(); i++) {
        cout << q.getAt(i) << " ";
    }
    cout << "\n";
}

void displayResources(CampusSystem& sys) {
    cout << "\n--- Resource Availability ---\n";
    if (sys.resourceCount == 0) {
        cout << "No resources registered.\n";
        return;
    }
    cout << "  " << left << setw(8) << "ID" << setw(20) << "Name" << setw(12) << "Type"
         << setw(14) << "Status" << setw(10) << "Request" << "Parcel" << "\n";
    cout << "  ----------------------------------------------------------------------\n";
    for (int i = 0; i < sys.resourceCount; i++) {
        Resource& res = sys.resources[i];
        string reqId = "-";
        string pId = "-";
        if (res.requestId != "") {
            reqId = res.requestId;
            int rIdx = findRequest(sys, res.requestId);
            if (rIdx != -1) {
                pId = sys.requests[rIdx].parcelId;
            }
        }
        cout << "  " << left << setw(8) << res.resourceId << setw(20) << res.name
             << setw(12) << res.type << setw(14) << res.status
             << setw(10) << reqId << pId << "\n";
    }
    printFreeQueue("Available counters (round-robin)     ", sys.counterQueue);
    printFreeQueue("Available delivery staff (round-robin)", sys.deliveryQueue);
}

void assignNextRequest(CampusSystem& sys) {
    cout << "\n--- Process Next Waiting Request ---\n";
    if (sys.requestQueue.isEmpty()) {
        cout << "No waiting requests.\n";
        return;
    }

    RequestNode* current = sys.requestQueue.getFront();
    int rIndex = -1;
    while (current != nullptr) {
        int index = findRequest(sys, current->requestId);
        if (index != -1) {
            string type = resourceTypeFor(sys.requests[index].serviceType);
            if (!queueFor(sys, type).isEmpty()) {
                rIndex = index;
                break;
            }
        }
        current = current->next;
    }

    if (rIndex == -1) {
        cout << "No available resources match the waiting requests right now.\n";
        return;
    }

    string type = resourceTypeFor(sys.requests[rIndex].serviceType);
    string resourceId = queueFor(sys, type).peek();
    doAssignment(sys, rIndex, findResource(sys, resourceId));
}

void assignChosenRequest(CampusSystem& sys) {
    cout << "\n--- Assign Request to Chosen Resource ---\n";
    string reqId = toUpper(readLine("Request ID : "));
    string resId = toUpper(readLine("Resource ID: "));
    int rIndex = findRequest(sys, reqId);
    int resIndex = findResource(sys, resId);

    if (rIndex == -1) {
        cout << "Error: Request ID not found.\n";
        return;
    }
    if (resIndex == -1) {
        cout << "Error: Resource ID not found.\n";
        return;
    }
    ServiceRequest& r = sys.requests[rIndex];
    Resource& res = sys.resources[resIndex];

    if (r.status != "Waiting") {
        cout << "Error: Request is already " << r.status << ".\n";
        return;
    }
    if (res.status != "Available") {
        cout << "Error: Resource is currently " << res.status << ".\n";
        return;
    }
    if (res.type != resourceTypeFor(r.serviceType)) {
        cout << "Error: " << r.serviceType << " requires a " << resourceTypeFor(r.serviceType)
             << " resource, but " << resId << " is " << res.type << ".\n";
        return;
    }
    doAssignment(sys, rIndex, resIndex);
}

void updateResourceStatus(CampusSystem& sys) {
    cout << "\n--- Update Resource Status ---\n";
    string id = toUpper(readLine("Resource ID: "));
    int resIndex = findResource(sys, id);
    if (resIndex == -1) {
        cout << "Error: Resource not found.\n";
        return;
    }
    Resource& res = sys.resources[resIndex];
    cout << "Resource " << res.resourceId << " (" << res.name << ") is currently " << res.status << "\n";
    cout << "  1. Set Available\n";
    cout << "  2. Set Unavailable (break / off duty)\n";
    cout << "  0. Back\n";
    int choice = readInt("Choose: ");

    if (choice == 1) {
        if (res.status == "Available") {
            cout << "Resource is already Available.\n";
            return;
        }
        if (res.status == "Occupied") {
            cout << "Cannot set available while occupied with a request.\n";
            return;
        }
        if (queueFor(sys, res.type).isFull()) {
            cout << "Error: " << res.type << " queue is full.\n";
            return;
        }
        res.status = "Available";
        queueFor(sys, res.type).enqueue(res.resourceId);
        cout << id << " is now Available.\n";
    }
    else if (choice == 2) {
        if (res.status == "Unavailable") {
            cout << "Resource is already Unavailable.\n";
            return;
        }
        if (res.status == "Available") {
            queueFor(sys, res.type).removeResource(res.resourceId);
            res.status = "Unavailable";
            cout << id << " marked as Unavailable.\n";
            return;
        }
        // if Occupied, free the request back to waiting
        string reqId = res.requestId;
        int rIndex = findRequest(sys, reqId);
        if (rIndex != -1) {
            int pIndex = findParcel(sys, sys.requests[rIndex].parcelId);
            if (pIndex != -1) {
                string oldStatus = sys.parcels[pIndex].status;

                res.status = "Unavailable";
                res.requestId = "";
                returnRequestToWaiting(sys, rIndex);

                logActivity(sys, sys.parcels[pIndex].parcelId, "RESOURCE",
                            id + " unavailable, " + reqId + " returned to waiting",
                            oldStatus, sys.parcels[pIndex].status, reqId, id, false);
                cout << id << " marked Unavailable. Request " << reqId << " returned to waiting queue.\n";
                return;
            }
        }
        res.status = "Unavailable";
        res.requestId = "";
        cout << id << " marked Unavailable.\n";
    }
}

void task3Menu(CampusSystem& sys) {
    int choice = -1;
    while (choice != 0) {
        cout << "\n========== TASK 3: RESOURCES & ALLOCATION ==========\n";
        cout << "  1. Register resource\n";
        cout << "  2. View resource availability\n";
        cout << "  3. Auto assign next request\n";
        cout << "  4. Manually assign request to resource\n";
        cout << "  5. Update resource status\n";
        cout << "  0. Back to main menu\n";
        choice = readInt("Choose: ");

        switch (choice) {
            case 1: registerResource(sys); break;
            case 2: displayResources(sys); break;
            case 3: assignNextRequest(sys); break;
            case 4: assignChosenRequest(sys); break;
            case 5: updateResourceStatus(sys); break;
            case 0: break;
            default: cout << "Invalid choice!\n";
        }
    }
}

#endif
