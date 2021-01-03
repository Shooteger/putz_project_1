//          Copyright Maurice Putz 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file boost_license.txt)

#include <iostream>
#include <thread>
#include <bitset>
#include <bits/stdc++.h>

#include "queue.h"
#include "CLI11.hpp"

using namespace std;
using namespace CLI;

ulong encode(char character_to_encode) {
    return bitset<8>(character_to_encode).to_ulong();
}



int main(int argc, char* argv[]) {

    Queue<ulong> q{};
    string input_chars;

    //thread sender{[], q.push(1) };
    //thread receiver{[], q.push(2) };

    //sender.join();
    //receiver.join();

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

    App app {"MLT-3 Encoding"};

    app.add_option("input_chars", input_chars, "Given input_chars to send over with MLT-3");
    
    try {
        CLI11_PARSE(app, argc, argv);
    } catch (exception& e) {
        cerr << e.what() << "\n";
    }

    sort(input_chars.begin(), input_chars.end()); 
    auto res = unique(input_chars.begin(), input_chars.end());
    input_chars = string(input_chars.begin(), res);

    for (size_t i=0; i < input_chars.length(); ++i) {
        q.push(encode(input_chars.at(i)));
    }

    cout << input_chars;
    cout << "\nPop: " << *q.pop_and_wait() << "\n";
    cout << "Pop: " << *q.pop_and_wait() << "\n";
    cout << "Pop: " << *q.pop_and_wait() << "\n";
    cout << "Pop: " << *q.pop_and_wait() << "\n";
    //cout << "Pop: " << *q.pop_and_wait() << "\n";
    //cout << "Pop: " << *q.pop_and_wait() << "\n";
    
}
