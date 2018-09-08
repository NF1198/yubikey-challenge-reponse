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

#include <stdint.h>
#include <string>
#include <iostream>
#include <boost/algorithm/hex.hpp>
#include "./byte_array.h"
#include "./LOG.h"
#include "./yubi.h"
#include "./prog_opts.h"

using namespace std;

int main(int argc, char *argv[])
{
    try
    {
        ProgramOptions opts = ProgramOptions(argc, argv);
        LibraryManager yklibrary = LibraryManager();
        try
        {
            std::unique_ptr<YubiKey> key(new YubiKey());
            LOG << "version: " << key->version() << endl;
            LOG << "firmware: " << key->firmwareVersion() << endl;
            LOG << "serial number: " << key->serialNumber() << endl;
            LOG << "challenge: " << opts.challenge << endl;

            byte_array unhexed;

            if (opts.hexInput)
            {
                try
                {
                    boost::algorithm::unhex(opts.challenge, std::back_inserter(unhexed));
                }
                catch (const std::exception &e)
                {
                    std::cerr << "Unable to parse challenge (non-hex input)" << endl;
                    return 1;
                }
            }
            else
            {
                unhexed = reinterpret_cast<const unsigned char *>(opts.challenge.c_str());
            }

            if (unhexed.length() > 64)
            {
                LOG << "warning: truncated challenge to 64 characters" << endl;
                unhexed.resize(64);
            }

            int len;
            const unsigned char *chal;

            len = unhexed.length();
            chal = unhexed.data();

            LOG << "length: " << len << endl;

            std::string resp;

            resp = key->challengeResponse(
                chal,
                len,
                opts.slot,
                opts.hmac);

            cout << resp << endl;
        }
        catch (const char *e)
        {
            LOG << "error: " << e << endl;
        }

        return 0;
    }
    catch (const char *reason)
    {
        LOG << reason << endl;
        return 1;
    }
}