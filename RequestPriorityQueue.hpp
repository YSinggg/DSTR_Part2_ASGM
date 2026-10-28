#ifndef REQUEST_PRIORITY_QUEUE_HPP
#define REQUEST_PRIORITY_QUEUE_HPP

#include <iostream>
#include <string>
using namespace std;

struct RequestNode {
    string requestId;
    int priority;
    int orderNo;
    RequestNode* next;
};

class RequestPriorityQueue {
private:
    RequestNode* front;
    int count;

public:
    RequestPriorityQueue() {
        front = nullptr;
        count = 0;
    }

    ~RequestPriorityQueue() {
        while (!isEmpty()) {
            dequeue();
        }
    }

    bool isEmpty() {
        return front == nullptr;
    }

    int size() {
        return count;
    }

    void enqueue(string reqId, int prio, int order) {
        RequestNode* newNode = new RequestNode;
        newNode->requestId = reqId;
        newNode->priority = prio;
        newNode->orderNo = order;
        newNode->next = nullptr;

        // lower priority value means higher urgency
        if (front == nullptr || prio < front->priority || 
           (prio == front->priority && order < front->orderNo)) {
            newNode->next = front;
            front = newNode;
        } else {
            RequestNode* curr = front;
            while (curr->next != nullptr && 
                  (curr->next->priority < prio || 
                  (curr->next->priority == prio && curr->next->orderNo <= order))) {
                curr = curr->next;
            }
            newNode->next = curr->next;
            curr->next = newNode;
        }
        count++;
    }

    string dequeue() {
        if (isEmpty()) return "";
        RequestNode* temp = front;
        string id = temp->requestId;
        front = front->next;
        delete temp;
        count--;
        return id;
    }

    string peek() {
        if (isEmpty()) return "";
        return front->requestId;
    }

    bool removeRequest(string reqId) {
        if (isEmpty()) return false;

        if (front->requestId == reqId) {
            dequeue();
            return true;
        }

        RequestNode* curr = front;
        while (curr->next != nullptr && curr->next->requestId != reqId) {
            curr = curr->next;
        }

        if (curr->next != nullptr) {
            RequestNode* toDelete = curr->next;
            curr->next = toDelete->next;
            delete toDelete;
            count--;
            return true;
        }
        return false;
    }

    RequestNode* getFront() {
        return front;
    }
};

#endif
