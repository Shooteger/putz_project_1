//          Copyright Maurice Putz 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file boost_license.txt)

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
        Queue(const Queue& other) { //only one thread can write at any point of interaction
            std::lock_guard lock(mtx);
            data_queue = other.data_queue;
        } 
        
        //for pushing a new element, locking queue, than pushing and notify
        void push(T value) { 
            std::lock_guard lock(mtx);
            data_queue.push(value);
            data_notify.notify_one();
        }
        
        //waits until it gets notified, so that it can start operating
        //if queue is not empty, it will copy the front value of the qeue and then pop
        void pop_and_wait(T& value) {
            std::unique_lock lock(mtx); //unique_lock and not lock_guard because more flexibility if data_queue is empty -> wait()
            data_notify.wait(lock, [this]{return !data_queue.empty();});
            value = data_queue.front();
            data_queue.pop();
        }

        //waits until it gets notified, so that it can start operating
        //if queue is not empty, it will copy the front value of the qeue and then pop
        //returns a pointer to poped element
        std::shared_ptr<const T> pop_and_wait() {
            std::unique_lock lock(mtx); //unique_lock and not lock_guard because more flexibility if data_queue is empty -> wait()
            data_notify.wait(lock, [this]{return !data_queue.empty();});
            std::shared_ptr<const T> res(std::make_shared<const T>(data_queue.front()));
            data_queue.pop();
            return res;
        }

        //returns 1 of queue is empty, 0 if not
        bool empty() {
            std::lock_guard lock(mtx);
            return data_queue.empty();
        }

    private:
        mutable std::mutex mtx;
        std::queue<T> data_queue;
        std::condition_variable data_notify;
};