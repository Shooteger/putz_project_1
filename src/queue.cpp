/* author: Maurice Putz
 * title: MLT3Sender
 * Cat. number: 16
*/

#include <condition_variable>
#include <mutex>
#include <iostream>
#include <queue>
#include <memory>

#include "queue.h"

using namespace std;

template <typename T>
void Queue<T>::push(T value) {
    lock_guard lock(mtx);
    data_queue.push(value);
    data_notify.notify_one();
}

/*
template <typename T>
void Queue<T>::pop(T& value) {
    unique_lock lock(mtx);
    data_notify.wait(lock, [this]{return !data_queue.empty();});
    value = data_queue.front();
    data_queue.pop();
}
*/

template <typename T>
bool Queue<T>::empty() {
    lock_guard lock(mtx);
    return data_queue.empty();
}