//          Copyright Maurice Putz 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file boost_license.txt)

#include <iostream>
#include <thread>
#include <bitset>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <chrono>

#include "queue.h"
#include "CLI11.hpp"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/async.h"
#include "rang.hpp"

//ignore warnings from extern library "tabulate"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#include "tabulate/tabulate.hpp"
#pragma GCC diagnostic pop

using namespace std;
using namespace tabulate;

string home = getenv("HOME");
string logPath = home;
auto logger = spdlog::basic_logger_mt<spdlog::async_factory>("async_file_logger", logPath.append("/Desktop/mlt3logs/log.txt")); //async logger -> faster

//for encoding the character if the input string to ascii DEC numbers
int encode(char character_to_encode) {
    return bitset<8>(character_to_encode).to_ulong();
}

//returns a random amount from 1 until 127 numbers of random numbers between 33 and 126
//this are the ASCII character in DEC and all special and alphanumerical character of ASCII
vector<int> random_tf(Table& tab, string allowed="") {
    vector<int> res;
    srand((int)time(0));
	int repeat = (rand() % (126-33)) + 1;   //how often random ascii sign should be repeated

    if (allowed == "") {
        int i = 0;
        while(i++ < repeat) {
            res.push_back((rand() % (126-33)) + 33); //33 is ! and 126 is ~ all between are numbers, special signs or letters
        }
    } else {
        vector<int> tmp_bitsets;
        for (size_t i=0; i < allowed.length(); ++i) {
            tmp_bitsets.push_back(encode(allowed.at(i)));   //get bitset for given ascii signs in allowed
        }

        int j = 0;
        while(j++ < repeat) {
            int random_idx = rand() % tmp_bitsets.size();
            res.push_back(bitset<8>(tmp_bitsets[random_idx]).to_ullong());    //access random elemnt of the new vector with allowed ascii signs from input
        }
    }
    string tmp;
    //table add all rows and col1 data
    for (size_t i=0; i < res.size(); ++i) {
        //tmp = char(res[i]);
        tmp = char(res[i]);
        tab.add_row({tmp, "", "", "", ""});
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
    int idx_cnt = 7;

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

string decrypt_from_mlt3(string mlt3_data, Table& tab, int table_col_cnt) {
    string encoded_res = "0";
    
    for (size_t i = 1; i < mlt3_data.size(); ++i) {
        if (mlt3_data.at(i) !=  mlt3_data.at(i-1)) {
            encoded_res += "1";
        } else {
            encoded_res += "0";
        }
    }

    //cout << table_col_cnt << "\n";
    tab[table_col_cnt][3].set_text(encoded_res); //SETS binary format table column4
    
    stringstream sstream(encoded_res);
    string output;
    while(sstream.good())
    {
        bitset<8> bits;
        sstream >> bits;
        char c = char(bits.to_ulong());
        output += c;
    }

    tab[table_col_cnt][4].set_text(output);  //dispaly ascii signs in last column 5
    return output;
}

//sets the DataPromise and Future from sender thread, that the receiver thread
//can pick up the sended data in binary format
void send_data_tf(vector<int> data_to_send, Queue<string>& queue, Table& tab) {
    vector<bitset<8>> help_vec_tmp;
    for (size_t i=0; i < data_to_send.size(); ++i) {
        help_vec_tmp.push_back((bitset<8> (data_to_send[i])));
        tab[i+1][1].set_text(help_vec_tmp[i].to_string());   //for second column output in table
    }
    //every bitblog will be converted with mlt3 and send to promise for receiver thread
    string res = "";
    for (size_t i=0; i < help_vec_tmp.size(); ++i) {
        string tmp = convert_to_mlt3(help_vec_tmp[i]);
        queue.push(tmp);
        tab[i+1][2].set_text(tmp);  //for third column in table
    }
    logger->info("All data succesfulls on queue pushed");
}

//sets the DataEncoded Promise and Future, so that the encoded data of the receiver thread can be printed
void decode(Queue<string>& queue, Table& tab) {
    int cnt = 1;
    string tmp;
    while (!queue.empty()) {
        tmp = decrypt_from_mlt3(*queue.pop_and_wait(), ref(tab), cnt);
        cnt++;
    }
}

int main(int argc, char* argv[]) {
    
    string input_chars;
    bool f = false;
    bool t = false;
    bool c = false;

    set<string> check_set = {"!", "\"", "#", "$", "%", "&", "'", "(", ")", "*", "+", ",", "-", ".", "/", "0",
                             "1", "2", "3", "4", "5", "6", "7", "8", "9", ":", ";", "<", "=", ">", "?", "@",
                             "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p",
                             "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "[", "\\", "]", "^", "_", "´",
                             "{", "}", "|", "~"};
    
    CLI::App app {"MLT-3 Encoding"};
    app.add_option("input_characters", input_chars,
         "Only given characters will be random times send over with MLT-3.    Example: \"./mlt3send asdf\"")->check(CLI::IsMember(check_set))->ignore_case();
    app.add_flag("-f,--file", f, "Writes the ouput of MLT-3 process an ASCII-Doc and outputs it on console as well");
    app.add_flag("-t,--time", t, "Time measurement of sending until receiving data");
    app.add_flag("-c,--color", c, "Standard table output with color");

    //NOTE ADD WHICH ASCII CHARACTERS ARE ALLOWED! 33 until 129 in dec!
    cout << rang::fg::red;
    try {
        CLI11_PARSE(app, argc, argv);
    } catch(const CLI::ParseError &e) {
        logger->error("Program terminated because of parse exception: {0}", e.what());
        return app.exit(e);
    }
    cout << rang::style::reset;

    Queue<string> q{}; //create queue

    // this block is for deleting double characters from input string
    //START
    sort(input_chars.begin(), input_chars.end()); 
    auto res = unique(input_chars.begin(), input_chars.end());
    input_chars = string(input_chars.begin(), res);
    //END

    Table main_table;   //create table for output

    main_table.add_row({"ASCII data to send", "Binary", "MTL-3", 
                        "Binary encoded from MLT-3", "Data received in ASCII"});

    if (c) {          
        main_table.format().corner_color(Color::magenta).border_color(Color::magenta)
        .font_style({FontStyle::bold, FontStyle::underline}).font_color(Color::cyan);
    }

    auto start = chrono::steady_clock::now();   //starts time measurement
    //thread to send and push data to queue gets started
    thread sender{send_data_tf, random_tf(ref(main_table), input_chars), ref(q), ref(main_table)};  //ref() for rvalue error in std::thread because its given by reference
    sender.join();

    //thread to decode and pop data from queue gets started
    thread receiver{decode, ref(q), ref(main_table)};
    receiver.join();
    auto end = chrono::steady_clock::now();   //ends time measurement

    if (!f) {   //if not -f flag
        cout << main_table << "\n";
        logger->info("table printed");
    }
    if (f) {    // if f, then asciidoc output on console and write into file
        MarkdownExporter exporter;
        //AsciiDocExporter exporter;
        //string asciidoc = exporter.dump(main_table);
        string mkdoc = exporter.dump(main_table);

        string tmp_path = home.append("/Desktop/mlt3.mkd");
        ofstream file;
        file.open(tmp_path, ios::out | ios::trunc | ios_base::binary);
        if (!file) {
            string err_msg = "Could not open file: ";
            logger->error(err_msg.append(tmp_path));
            cout << "Could not create file at path: " << tmp_path << "\n";
        } else {
            cout << "\n" << mkdoc;
            //asciidoc.erase(remove(asciidoc.begin(), asciidoc.end(), '\00'), asciidoc.end());
            file << mkdoc;
            file.close();
            cout << rang::fg::magenta << "\n\n[NOTE]\nThe result is saved to the location: "
                 << rang::style::underline << rang::fg::cyan << tmp_path
                 << rang::style::reset << rang::fg::magenta << "\nIt is formated as markdown.\n" << rang::style::reset;
            /*
            cout << rang::fg::magenta << "\n\n[NOTE]\nYou can render your ASCII Document with an ASCII Rendertool or online at: \n"
                 << rang::style::underline << rang::fg::cyan <<"https://www.tutorialspoint.com/online_asciidoc_editor.php\n"
                 << rang::style::reset << rang::fg::magenta << "\nYou can copy and paste the output on the console or the content of the file into the page"
                 << "on the left side of the window and then press \"preview\", to see the rendered table.\n\n" 
                 << rang::style::reset;
            */
            logger->info("write in file succesfull");
        }
    }

    if (t) {
        string t_measure = to_string(chrono::duration_cast<chrono::nanoseconds>(end - start).count());
        cout << "\nElapsed time in nanoseconds  : " << t_measure << " ns\n";
        cout << "Elapsed time in microseconds : " << chrono::duration_cast<chrono::microseconds>(end - start).count() << " µs\n";
        string tmp = "time of threads elapsed: ";
        logger->info(tmp.append(t_measure));
    }
}