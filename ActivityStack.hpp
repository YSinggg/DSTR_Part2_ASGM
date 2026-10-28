#ifndef ACTIVITY_STACK_HPP
#define ACTIVITY_STACK_HPP

#include <iostream>
#include <string>
#include "Models.hpp"
using namespace std;

struct ActivityNode {
    Activity data;
    ActivityNode* next;
};

class ActivityStack {
private:
    ActivityNode* top;
    int count;

public:
    ActivityStack() {
        top = nullptr;
        count = 0;
    }

    ~ActivityStack() {
        while (!isEmpty()) {
            pop();
        }
    }

    bool isEmpty() {
        return top == nullptr;
    }

    int size() {
        return count;
    }

    void push(Activity a) {
        ActivityNode* newNode = new ActivityNode;
        newNode->data = a;
        newNode->next = top;
        top = newNode;
        count++;
    }

    bool pop() {
        if (isEmpty()) return false;
        ActivityNode* temp = top;
        top = top->next;
        delete temp;
        count--;
        return true;
    }

    Activity* peek() {
        if (isEmpty()) return nullptr;
        return &top->data;
    }

    ActivityNode* getTop() {
        return top;
    }

    Activity* findActivity(int no) {
        ActivityNode* curr = top;
        while (curr != nullptr) {
            if (curr->data.activityNo == no) {
                return &curr->data;
            }
            curr = curr->next;
        }
        return nullptr;
    }

    int latestActivityFor(string parcelId) {
        ActivityNode* curr = top;
        while (curr != nullptr) {
            if (curr->data.parcelId == parcelId && !curr->data.reversed && curr->data.type != "UNDO") {
                return curr->data.activityNo;
            }
            curr = curr->next;
        }
        return -1;
    }
};

#endif
