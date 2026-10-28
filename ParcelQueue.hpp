#ifndef PARCEL_QUEUE_HPP
#define PARCEL_QUEUE_HPP

#include <iostream>
#include <string>
using namespace std;

struct ParcelNode {
    string parcelId;
    ParcelNode* next;
};

class ParcelQueue {
private:
    ParcelNode* front;
    ParcelNode* rear;
    int count;

public:
    ParcelQueue() {
        front = nullptr;
        rear = nullptr;
        count = 0;
    }

    ~ParcelQueue() {
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

    void enqueue(string parcelId) {
        ParcelNode* newNode = new ParcelNode;
        newNode->parcelId = parcelId;
        newNode->next = nullptr;

        if (isEmpty()) {
            front = newNode;
            rear = newNode;
        } else {
            rear->next = newNode;
            rear = newNode;
        }
        count++;
    }

    string dequeue() {
        if (isEmpty()) {
            return "";
        }
        ParcelNode* temp = front;
        string id = temp->parcelId;
        front = front->next;
        if (front == nullptr) {
            rear = nullptr;
        }
        delete temp;
        count--;
        return id;
    }

    string peek() {
        if (isEmpty()) return "";
        return front->parcelId;
    }

    bool removeParcel(string parcelId) {
        if (isEmpty()) return false;

        if (front->parcelId == parcelId) {
            dequeue();
            return true;
        }

        ParcelNode* curr = front;
        while (curr->next != nullptr && curr->next->parcelId != parcelId) {
            curr = curr->next;
        }

        if (curr->next != nullptr) {
            ParcelNode* toDelete = curr->next;
            curr->next = toDelete->next;
            if (toDelete == rear) {
                rear = curr;
            }
            delete toDelete;
            count--;
            return true;
        }
        return false;
    }

    ParcelNode* getFront() {
        return front;
    }
};

#endif
