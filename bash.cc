/**
 * @file bash.cc
 * @author phanen
 * @brief a tiny bash made for tiny-fat kernel
 * @date 2022-05-31
 *
 * @copyright Copyright (c) 2022. phanen
 *
 */

#include "kernel.h"
#include <iostream>
#include <sstream>
using namespace std;

void bash_main()
{
    string input;
    stringstream ss;
    //  main loop
    while (getline(cin, input))
    {
        // whole line into ss
        ss << input;
        // back a word
        ss >> input;

        if (input == "exit")
        {
            break;
        }
        else if (input == "mkdir")
        {
            while (ss >> input)
            {
                fs_create(input.c_str(), TYPE_DIR);
            }
        }
        else if (input == "touch")
        {
            while (ss >> input)
            {
                fs_create(input.c_str(), TYPE_FILE);
            }
        }
        else if (input == "ls")
        {
            fs_ls();
        }
        else if (input == "cd")
        {
            ss >> input;
            fs_cd(input.c_str());
        }
        else if (input == "rm")
        {
            while (ss >> input)
                fs_delete(input.c_str());
        }
        else if (input == "cat")
        {
            ss >> input;
            char buf[1000]{};
            int fd = fs_open(input.c_str());
            int len = fs_read(fd, buf, 128); // read as mush as possible
            buf[len] = '\0';
            cout << buf << endl;
            fs_close(fd);
        }
        else if (input == "wrt")
        {
            ss >> input;
            int fd = fs_open(input.c_str());
            // edit the content form terminal the write in
            // support one line only....
            getline(cin, input);
            int len = fs_write(fd, input.c_str(), input.size());
            fs_close(fd);
        }
        else
        {
            cout << "no such cmd" << endl;
        }

        ss.clear();
        ss.str("");
    } // end main loop
    return;
}
