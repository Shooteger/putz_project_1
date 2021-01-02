/* author: Maurice Putz
 * title: MLT3Sender
 * Cat. number: 16
*/

#pragma once

#include <condition_variable>
#include <mutex>
#include <iostream>
#include <queue>
#include <memory>

template <typename T>
class Queue {
    public:
        Queue() {};
        Queue(const Queue& other) { //only one thread can write at any point
            std::lock_guard lock(mtx);
            data_queue = other.data_queue;
        } 
        void push(T value);
        void pop(T& value);
        bool empty();
    private:
        mutable std::mutex mtx;
        std::queue<T> data_queue;
        std::condition_variable data_notify;
};