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
#include <ostream>
#include <string>
#include <sstream>
#include <boost/algorithm/hex.hpp>
#include <ykpers-1/ykcore.h>
#include <ykpers-1/ykstatus.h>
#include <ykpers-1/ykdef.h>

#include "./LOG.h"
#include "./yubi.h"

using namespace std;

#define yk_errstr (yk_errno) ? yk_strerror(yk_errno) : ""

void handle_yk_error()
{
    int err = yk_errno;
    if (err == YK_EUSBERR)
    {
        LOG << "USB error: " << yk_usb_strerror() << endl;
    }
    else
    {
        LOG << yk_strerror(err);
    }
}

LibraryManager::LibraryManager()
{
    LOG << "initializing yubikey library..." << endl;
    yk_init();
}
LibraryManager::~LibraryManager()
{
    LOG << "releasing yubikey library..." << endl;
    yk_release();
}



YubiKey::YubiKey()
{
    this->key_ = yk_open_first_key();
    if (!key_)
    {
        throw(yk_errstr);
    }
    this->status_ = ykds_alloc();
    yk_get_status(this->key_, this->status_);
    {
        LOG << "opened key: " << this->serialNumber() << endl;
    }
}

YubiKey::~YubiKey()
{
    if (this->status_)
    {
        ykds_free(this->status_);
    }
    if (this->key_)
    {
        LOG << "closing key..." << endl;
        yk_close_key(this->key_);
    }
}

int YubiKey::versionMajor()
{
    return (this->status_) ? ykds_version_major(this->status_) : 0;
}

int YubiKey::versionMinor()
{
    return (this->status_) ? ykds_version_minor(this->status_) : 0;
}

int YubiKey::versionBuild()
{
    return (this->status_) ? ykds_version_build(this->status_) : 0;
}

const std::string YubiKey::version()
{
    std::ostringstream verstr;
    verstr << this->versionMajor() << "." << this->versionMinor() << "." << this->versionBuild();
    std::string result = verstr.str();
    return result;
}

int YubiKey::firmwareVersion()
{
    return (this->key_ && this->status_) ? yk_check_firmware_version2(this->status_) : 0;
}

unsigned int YubiKey::serialNumber()
{
    if (this->versionMajor() >= 2)
    {
        unsigned int serial;
        yk_get_serial(this->key_, 0, 0, &serial);
        return serial;
    }
    return 0;
}

std::string YubiKey::challengeResponse(const unsigned char *challenge, int length, int slot, bool hmac)
{
    if (!this->key_)
    {
        throw("No yubikey detected");
    }
    uint8_t yk_cmd;
    switch (slot)
    {
    case 1:
        yk_cmd = (hmac == true) ? SLOT_CHAL_HMAC1 : SLOT_CHAL_OTP1;
        break;
    case 2:
        yk_cmd = (hmac == true) ? SLOT_CHAL_HMAC2 : SLOT_CHAL_OTP2;
        break;
    default:
        throw("invalid slot");
        break;
    }

    const unsigned char *chal = challenge;
    unsigned char resp[64];
    memset(resp, 0, sizeof(resp));

    if (!yk_challenge_response(this->key_, yk_cmd, 1, length,
                               chal, sizeof(resp), resp))
    {
        throw(yk_errstr);
    }

    std::string responseString(reinterpret_cast<char *>(resp), 0, 20);
    return boost::algorithm::hex(responseString);
}

