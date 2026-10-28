#ifndef RESOURCE_CIRCULAR_QUEUE_HPP
#define RESOURCE_CIRCULAR_QUEUE_HPP

#include <iostream>
#include <string>
using namespace std;

const int QUEUE_CAPACITY = 10;

class ResourceCircularQueue {
private:
    string items[QUEUE_CAPACITY];
    int front;
    int rear;
    int count;

public:
    ResourceCircularQueue() {
        front = 0;
        rear = -1;
        count = 0;
    }

    bool isEmpty() {
        return count == 0;
    }

    bool isFull() {
        return count == QUEUE_CAPACITY;
    }

    int size() {
        return count;
    }

    bool enqueue(string id) {
        if (isFull()) {
            return false;
        }
        rear = (rear + 1) % QUEUE_CAPACITY;
        items[rear] = id;
        count++;
        return true;
    }

    string dequeue() {
        if (isEmpty()) return "";
        string id = items[front];
        front = (front + 1) % QUEUE_CAPACITY;
        count--;
        return id;
    }

    string peek() {
        if (isEmpty()) return "";
        return items[front];
    }

    bool removeResource(string id) {
        bool found = false;
        int total = count;
        for (int i = 0; i < total; i++) {
            string current = dequeue();
            if (current == id && !found) {
                found = true;
            } else {
                enqueue(current);
            }
        }
        return found;
    }

    string getAt(int index) {
        return items[(front + index) % QUEUE_CAPACITY];
    }
};

#endif
