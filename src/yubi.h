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

#ifndef __YUBI_SUPPORT_CLASSES
#define __YUBI_SUPPORT_CLASSES

#include <string>
#include <ykpers-1/ykcore.h>


struct LibraryManager
{
    LibraryManager();

    ~LibraryManager();
};


class YubiKey
{
  private:
    YK_KEY *key_;
    YK_STATUS *status_;

  public:
    YubiKey();
    
    ~YubiKey();

    int versionMajor();

    int versionMinor();

    int versionBuild();

    const std::string version();

    int firmwareVersion();

    unsigned int serialNumber();

    std::string challengeResponse(const unsigned char *challenge, int length, int slot, bool hmac);
};

#endif