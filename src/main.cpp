//          Copyright Maurice Putz 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file boost_license.txt)

#include <iostream>
#include <thread>

#include "queue.h"
#include "CLI11.hpp"

using namespace std;
using namespace CLI;

int main(int argc, char* argv[]) {

    Queue<int> q{};

    //thread sender{[], q.push(1) };
    //thread receiver{[], q.push(2) };

    //sender.join();
    //receiver.join();

    App app {"MLT-3 Encoding"};

    CLI11_PARSE(app, argc, argv);
    
    //FOR QUEUE TESTING
    /*
    cout << q.empty() << "\n";
    q.push(10);
    q.push(20);
    cout << q.empty() << "\n";
    cout << "Pop: " << *q.pop_and_wait() << "\n";
    cout << "Pop: " << *q.pop_and_wait() << "\n";
    cout << q.empty() << "\n";
    */


}
