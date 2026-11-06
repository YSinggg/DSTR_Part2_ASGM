#ifndef CAMPUS_SYSTEM_HPP
#define CAMPUS_SYSTEM_HPP

#include <iostream>
#include <iomanip>
#include <string>
#include <ctime>
#include <cstdlib>
#include "Models.hpp"
#include "ParcelQueue.hpp"
#include "RequestPriorityQueue.hpp"
#include "ResourceCircularQueue.hpp"
#include "ActivityStack.hpp"

using namespace std;

struct CampusSystem {
    Parcel parcels[MAX_PARCELS];
    int parcelCount = 0;

    ServiceRequest requests[MAX_REQUESTS];
    int requestCount = 0;

    Resource resources[MAX_RESOURCES];
    int resourceCount = 0;

    ParcelQueue intakeQueue;
    RequestPriorityQueue requestQueue;
    ResourceCircularQueue counterQueue;
    ResourceCircularQueue deliveryQueue;
    ActivityStack history;

    int nextParcelNo = 1;
    int nextRequestNo = 1;
    int nextActivityNo = 1;
};

// Input helper functions
string trim(string s) {
    int start = 0;
    while (start < (int)s.length() && (s[start] == ' ' || s[start] == '\t' || s[start] == '\r' || s[start] == '\n')) {
        start++;
    }
    int end = (int)s.length() - 1;
    while (end >= start && (s[end] == ' ' || s[end] == '\t' || s[end] == '\r' || s[end] == '\n')) {
        end--;
    }
    if (end < start) return "";
    return s.substr(start, end - start + 1);
}

string toUpper(string text) {
    for (int i = 0; i < (int)text.length(); i++) {
        if (text[i] >= 'a' && text[i] <= 'z') {
            text[i] = text[i] - 32;
        }
    }
    return text;
}

string readLine(string prompt) {
    cout << prompt;
    string input;
    if (!getline(cin, input)) {
        return "";
    }
    return trim(input);
}

int readInt(string prompt) {
    string str;
    while (true) {
        cout << prompt;
        if (!getline(cin, str)) {
            exit(0);
        }
        str = trim(str);
        try {
            return stoi(str);
        } catch (...) {
            cout << "Invalid input! Please enter a number.\n";
        }
    }
}

string makeId(string prefix, int num) {
    if (num < 10) return prefix + "00" + to_string(num);
    if (num < 100) return prefix + "0" + to_string(num);
    return prefix + to_string(num);
}

string currentTime() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    char buf[30];
    strftime(buf, sizeof(buf), "%d/%m/%Y %H:%M", ltm);
    return string(buf);
}

// Search helpers (case-insensitive & whitespace trimmed)
int findParcel(CampusSystem& sys, string parcelId) {
    string target = toUpper(trim(parcelId));
    for (int i = 0; i < sys.parcelCount; i++) {
        if (toUpper(sys.parcels[i].parcelId) == target) {
            return i;
        }
    }
    return -1;
}

int findParcelByTracking(CampusSystem& sys, string trackingNo) {
    string target = toUpper(trim(trackingNo));
    for (int i = 0; i < sys.parcelCount; i++) {
        if (toUpper(sys.parcels[i].trackingNo) == target) {
            return i;
        }
    }
    return -1;
}

int findRequest(CampusSystem& sys, string requestId) {
    string target = toUpper(trim(requestId));
    for (int i = 0; i < sys.requestCount; i++) {
        if (toUpper(sys.requests[i].requestId) == target) {
            return i;
        }
    }
    return -1;
}

int findResource(CampusSystem& sys, string resourceId) {
    string target = toUpper(trim(resourceId));
    for (int i = 0; i < sys.resourceCount; i++) {
        if (toUpper(sys.resources[i].resourceId) == target) {
            return i;
        }
    }
    return -1;
}

bool isParcelFinished(string status) {
    if (status == "Collected" || status == "Delivered") return true;
    if (status == "Returned to Sender" || status == "Cancelled") return true;
    return false;
}

int priorityFor(string serviceType) {
    if (serviceType == "Special Handling") return 1;
    if (serviceType == "Re-delivery") return 2;
    if (serviceType == "Delivery") return 3;
    if (serviceType == "Collection") return 4;
    return 5;
}

string resourceTypeFor(string serviceType) {
    if (serviceType == "Collection" || serviceType == "Return to Sender") {
        return "Counter";
    }
    return "Delivery";
}

string waitingStatusFor(string serviceType) {
    if (serviceType == "Collection") return "Awaiting Collection";
    if (serviceType == "Delivery") return "Awaiting Delivery";
    if (serviceType == "Special Handling") return "Awaiting Special Handling";
    if (serviceType == "Re-delivery") return "Awaiting Re-delivery";
    return "Awaiting Return";
}

void logActivity(CampusSystem& sys, string parcelId, string type, string desc,
                 string oldStatus, string newStatus, string reqId, string resId,
                 bool canUndo) {
    Activity a;
    a.activityNo = sys.nextActivityNo++;
    a.time = currentTime();
    a.parcelId = parcelId;
    a.type = type;
    a.description = desc;
    a.oldStatus = oldStatus;
    a.newStatus = newStatus;
    a.requestId = reqId;
    a.resourceId = resId;
    a.canUndo = canUndo;
    a.reversed = false;
    sys.history.push(a);
}

int createServiceRequest(CampusSystem& sys, string parcelId, string serviceType);
void releaseResource(CampusSystem& sys, string resourceId);
void assignNextRequest(CampusSystem& sys);

#endif
