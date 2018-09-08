/* Copyright (C) Nicholas Folse, 2018 - All Rights Reserved
 * 
 * Written by Nicholas Folse <https://github.com/nf1198>
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the
 * Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to
 * do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall
 * be included in all copies or substantial portions of the
 * Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
 * KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
 * OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE
 */

#include <memory>
#include <exception>
#include <iostream>
#include <iomanip>
#include <ostream>
#include <string>
#include <sstream>
#include <boost/program_options.hpp>

#include "./LOG.h"
#include "./prog_opts.h"

using namespace std;
namespace po = boost::program_options;

ProgramOptions::ProgramOptions() {}

ProgramOptions::ProgramOptions(int argc, char *argv[]) {
    this->parse(argc, argv);
}

void ProgramOptions::parse(int argc, char *argv[])
{
    po::options_description desc("Allowed options");
    desc.add_options()(
        "help,h", "produce help message")(
        "slot,s", po::value<int>(&slot)->default_value(1), "yubikey slot")(
        "hmac,h", po::value<bool>(&hmac)->default_value(true), "true=HMAC; false=YubicoOTP (default = true)")(
        "hex,x", "input is hex encoded (default = false)")("verbose,v", "verbose output");

    po::options_description hidden("hidden options");
    hidden.add_options()(
        "challenge", po::value<std::string>(&challenge)->default_value(""), "challenge");

    po::options_description cmd_line_options("command line options");
    cmd_line_options.add(desc).add(hidden);

    po::positional_options_description p;
    p.add("challenge", 1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(cmd_line_options).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help"))
    {
        cout << "Usage:" << endl
             << endl;
        cout << "$ ykchallenge [options] [challenge]" << endl
             << endl;
        cout << desc << endl;
        throw("Program Terminated");
    }

    if (vm.count("hex"))
    {
        hexInput = true;
    }

    if (vm.count("verbose") == 0)
    {
        LOG_TARGET = null_stream;
    }

    if (slot < 1 || slot > 2)
    {
        throw(std::invalid_argument("Invalid slot"));
    }

    if (challenge == "-") {
        cin >> setw(128) >> challenge;
    }

    if (challenge.length() > 128)
    {
        throw(std::invalid_argument("Challenge should be max 64 characters"));
    }
}
