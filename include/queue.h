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
        
        void push(T value) {
            std::lock_guard lock(mtx);
            data_queue.push(value);
            data_notify.notify_one();
        }
        
        void pop_and_wait(T& value) {
            std::unique_lock lock(mtx);
            data_notify.wait(lock, [this]{return !data_queue.empty();});
            value = data_queue.front();
            data_queue.pop();
        }

        bool empty() {
            std::lock_guard lock(mtx);
            return data_queue.empty();
        }
    private:
        mutable std::mutex mtx;
        std::queue<T> data_queue;
        std::condition_variable data_notify;
};