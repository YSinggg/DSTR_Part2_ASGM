#ifndef MODELS_HPP
#define MODELS_HPP

#include <string>
using namespace std;

const int MAX_PARCELS = 200;
const int MAX_REQUESTS = 200;
const int MAX_RESOURCES = 10;

struct Parcel {
    string parcelId;
    string trackingNo;
    string recipientName;
    string recipientId;
    string location;
    string courier;
    string parcelType;
    string arrivalTime;
    string status;
    string activeRequestId;
};

struct ServiceRequest {
    string requestId;
    string parcelId;
    string serviceType;
    int priority;
    int orderNo;
    string requestTime;
    string status;
    string resourceId;
};

struct Resource {
    string resourceId;
    string name;
    string type;
    string status;
    string requestId;
};

struct Activity {
    int activityNo;
    string time;
    string parcelId;
    string type;
    string description;
    string oldStatus;
    string newStatus;
    string requestId;
    string resourceId;
    bool canUndo;
    bool reversed;
};

#endif
