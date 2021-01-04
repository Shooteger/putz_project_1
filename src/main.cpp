//          Copyright Maurice Putz 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file boost_license.txt)

#include <iostream>
#include <thread>
#include <bitset>
#include <bits/stdc++.h>
#include <future>

#include "queue.h"
#include "CLI11.hpp"

using namespace std;
using namespace CLI;

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
    return res;
}

//converts given binary block of 8 bit to mlt3 conform block
//and returns it as a string
string convert_to_mlt3(bitset<8> binary_block) {
    string res = "";
    string status = "000";

    cout << "binary: " << binary_block.to_string() << "; ";
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
        //cout << status: " << status.at(2) << ";; ";
        //cout << "idx: " << binary_block[i] << "; ";
        idx_cnt--;
    }
    cout << "MLT3: " << res << "; ";
    return res;
}

//sets the DataPromise and Future from sender thread, that the receiver thread
//can pick up the sended data in binary format
void send_data(promise<string>&& DataPromise, vector<short> data_to_send) {
    vector<bitset<8>> help_vec_tmp;
    for (size_t i=0; i < data_to_send.size(); ++i) {
        help_vec_tmp.push_back((bitset<8> (data_to_send[i])));
    }

    string res = "";
    for (size_t i=0; i < help_vec_tmp.size(); ++i) {
        res += convert_to_mlt3(help_vec_tmp[i]);
    }

    DataPromise.set_value(res);
}

//sets the DataEncoded Promise and Future, so that the encoded data of the receiver thread can be printed
void decode(promise<string>&& DataEncoded, string sended_data) {
    vector<bitset<8>> res_encoded;
    for (size_t i=0; i < sended_data.size(); ++i) {
        res_encoded.push_back((bitset<8> (sended_data[i])));
    }
    string res = "";
    DataEncoded.set_value(res);
}

int main(int argc, char* argv[]) {

    Queue<short> q{};
    string input_chars;
    //future and promise for sending data
    promise<string> DataToSend;
    future<string> DataFuture = DataToSend.get_future();

    //future and promise for printing sended data
    promise<string>  EncodedData;
    future<string> EncodedFuture = EncodedData.get_future();

    App app {"MLT-3 Encoding"};

    app.add_option("input_chars", input_chars, "Given input_chars to send over with MLT-3");

    //NOTE ADD WHICH ASCII CHARACTERS ARE ALLOWED! 33 until 129 in dec!
    CLI11_PARSE(app, argc, argv);

    // this block is for deleting double characters from input string
    //START
    sort(input_chars.begin(), input_chars.end()); 
    auto res = unique(input_chars.begin(), input_chars.end());
    input_chars = string(input_chars.begin(), res);
    //END

    //vector with random ascii values between 32 and 126
    auto random_digits = random(input_chars);    

    thread sender{send_data, move(DataToSend), random_digits};
    auto binary_sended_data = DataFuture.get(); //sets future for receiver thread with mlt3 encoded data

    //cout << binary_sended_data;

    thread receiver{decode, move(EncodedData), binary_sended_data};
    
    /*
    auto encoded_ascii_data = EncodedFuture.get();
    for (size_t i=0; i < encoded_ascii_data.size(); ++i) {
        cout << encoded_ascii_data[i] << "\n";
    }
    */

    sender.join();
    receiver.join();
}
