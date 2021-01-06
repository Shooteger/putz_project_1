//          Copyright Maurice Putz 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file boost_license.txt)

#include <iostream>
#include <thread>
#include <bitset>
#include <stdio.h>
#include <stdlib.h>
#include <future>


#include "queue.h"
#include "CLI11.hpp"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/async.h"
#include "tabulate/tabulate.hpp"

using namespace std;
using namespace CLI;
using namespace tabulate;

//const auto logger = spdlog::basic_logger_mt("basic_logger", "logs/basic-log.txt");  //basic logger
auto logger = spdlog::basic_logger_mt<spdlog::async_factory>("async_file_logger", "logs/async_log.txt"); //async logger -> faster

//for encoding the character if the input string to ascii DEC numbers
short encode(char character_to_encode) {
    return bitset<8>(character_to_encode).to_ulong();
}

//returns a random amount from 1 until 127 numbers of random numbers between 33 and 126
//this are the ASCII character in DEC and all special and alphanumerical character of ASCII
vector<short> random(string allowed="") {
    vector<short> res;
    srand((int)time(0));
	int repeat = (rand() % (126-33)) + 1;   //how often random ascii sign should be repeated

    if (allowed == "") {
        int i = 0;
        while(i++ < repeat) {
            res.push_back((rand() % (126-33)) + 33); //33 is ! and 126 is ~ all between are numbers, special signs or letters
        }
    } else {
        vector<short> tmp_bitsets;
        for (size_t i=0; i < allowed.length(); ++i) {
            tmp_bitsets.push_back(encode(allowed.at(i)));   //get bitset for given ascii signs in allowed
        }

        int j = 0;
        while(j++ < repeat) {
            int random_idx = rand() % tmp_bitsets.size();
            res.push_back(bitset<8>(tmp_bitsets[random_idx]).to_ullong());    //access random elemnt of the new vector with allowed ascii signs from input
        }
    }
    for (size_t i=0; i < res.size(); ++i) {
        cout << "Data to Decode and send: " << res[i] << "\n";
    }
    return res;
}

//converts given binary block of 8 bit to mlt3 conform block
//and returns it as a string, used in send_data(Promise, vecotor) function
string convert_to_mlt3(bitset<8> binary_block) {
    string res = "";
    string status = "000";
    
    //size is always 8 of given bit, return iteration because bitset
    //at index 0 ist first bit (first of right) of the byte
    short idx_cnt = 7;

    while (idx_cnt >= 0) {
        //cout << ;
        if (binary_block.test(idx_cnt)) {
            if (status == "000") {
                status = "00+";
            } else if (status == "00+") {
                status = "0+0";
            } else if (status == "0+0") {
                status = "+0-";
            } else if (status == "+0-") {
                status = "0-0";
            } else if (status == "0-0") {
                status = "-0+";
            } else if (status == "-0+") {
                status = "0+0";
            }
        } 
        res += status.at(2);
        idx_cnt--;
    }
    return res;
}

string decrypt_from_mlt3(string mlt3_data) {
    string encoded_res = "0";
    
    for (size_t i = 1; i < mlt3_data.size(); ++i) {
        if (mlt3_data.at(i) !=  mlt3_data.at(i-1)) {
            encoded_res += "1";
        } else {
            encoded_res += "0";
        }
    }
    stringstream sstream(encoded_res);
    string output;
    while(sstream.good())
    {
        bitset<8> bits;
        sstream >> bits;
        char c = char(bits.to_ulong());
        output += c;
    }
    return output;
}

//sets the DataPromise and Future from sender thread, that the receiver thread
//can pick up the sended data in binary format
void send_data(vector<short> data_to_send, Queue<string>& queue) {
    vector<bitset<8>> help_vec_tmp;
    for (size_t i=0; i < data_to_send.size(); ++i) {
        help_vec_tmp.push_back((bitset<8> (data_to_send[i])));
    }
    //every bitblog will be converted with mlt3 and send to promise for receiver thread
    string res = "";
    for (size_t i=0; i < help_vec_tmp.size(); ++i) {
        queue.push(convert_to_mlt3(help_vec_tmp[i]));
    }
    logger->info("All data succesfulls on queue pushed");
}

//sets the DataEncoded Promise and Future, so that the encoded data of the receiver thread can be printed
void decode(Queue<string>& queue) {
    while (!queue.empty()) {
        cout << "Encoded: " << decrypt_from_mlt3(*queue.pop_and_wait()) << ";\n";
    }
}

int main(int argc, char* argv[]) {
    
    string input_chars;

    App app {"MLT-3 Encoding"};
    app.add_option("input_characters", input_chars, "Only given characters will be random times send over with MLT-3.    Example: \"./mlt3send asdf\"");
    auto f = app.add_flag("-f,--file", "Writes every step of the process into a file");
    auto t = app.add_flag("-t,--table", "Output whole process as table formatted on the command line");
    auto s = app.add_flag("-s,--start", "Stop time between input and start of sending until receiving data");

    //NOTE ADD WHICH ASCII CHARACTERS ARE ALLOWED! 33 until 129 in dec!

    try {
        CLI11_PARSE(app, argc, argv);
    } catch(const ParseError &e) {
        logger->error("Program terminated because of parse exception: {0}", e.what());
        return app.exit(e);
    }

    Queue<string> q{};

    // this block is for deleting double characters from input string
    //START
    sort(input_chars.begin(), input_chars.end()); 
    auto res = unique(input_chars.begin(), input_chars.end());
    input_chars = string(input_chars.begin(), res);
    //END

    if (f) {

        logger->info("write in file succesfull");
    }
    if (t) {
        Table universal_constants;
        universal_constants.add_row({"Data to send", "Binary format", "MTL-3 format", "Binary format", "Data recceived"});

        cout << universal_constants << "\n";
        logger->debug("table printed");
    }

    if (s) {
        logger->info("");
    }

    thread sender{send_data, random(input_chars), ref(q)};  //ref() for rvalue error in std::thread because its given by reference
    sender.join();

    thread receiver{decode, ref(q)};
    receiver.join();
}