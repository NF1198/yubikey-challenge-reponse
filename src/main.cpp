
#include <memory>
#include <exception>
#include <iostream>
#include <ostream>
#include <string>
#include <sstream>
#include <boost/program_options.hpp>
#include <boost/algorithm/hex.hpp>
#include <ykpers-1/ykcore.h>
#include <ykpers-1/ykstatus.h>
#include <ykpers-1/ykdef.h>

using namespace std;
namespace po = boost::program_options;

class NullBuffer : public std::streambuf
{
  public:
    int overflow(int c) { return c; }
};

NullBuffer null_buffer;
std::ostream null_stream(&null_buffer);

//std::ostream &LOG = null_stream;
std::ostream &LOG = std::cout;

struct ProgramOptions
{
    int slot = 1;
    bool hmac = true;
    bool hexInput = false;
    std::string challenge;
    void parse(int argc, char *argv[])
    {
        po::options_description desc("Allowed options");
        desc.add_options()("help,h", "produce help message")("slot,s", po::value<int>(&slot)->default_value(1), "yubikey slot")("hmac,h", po::value<bool>(&hmac)->default_value(true), "true=HMAC; false=YubicoOTP (default = true)")("hex,x", "input is hex encoded (default = false)")("verbose,v", "verbose output");

        po::options_description hidden("hidden options");
        hidden.add_options()("challenge", po::value<std::string>(&challenge)->default_value(""), "challenge");

        po::options_description cmd_line_options("command line options");
        cmd_line_options.add(desc).add(hidden);

        po::positional_options_description p;
        p.add("challenge", -1);

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

        if (vm.count("verbose"))
        {
        }

        if (slot < 1 || slot > 2)
        {
            throw("Invalid slot");
        }
    }
};

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

class YubiKey
{
  public:
    YK_KEY *key_;
    YK_STATUS *status_;
    YubiKey()
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

  public:
    ~YubiKey()
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

    int versionMajor()
    {
        return (this->status_) ? ykds_version_major(this->status_) : 0;
    }

    int versionMinor()
    {
        return (this->status_) ? ykds_version_minor(this->status_) : 0;
    }

    int versionBuild()
    {
        return (this->status_) ? ykds_version_build(this->status_) : 0;
    }

    const std::string version()
    {
        std::ostringstream verstr;
        verstr << this->versionMajor() << "." << this->versionMinor() << "." << this->versionBuild();
        std::string result = verstr.str();
        return result;
    }

    int firmwareVersion()
    {
        return (this->key_ && this->status_) ? yk_check_firmware_version2(this->status_) : 0;
    }

    unsigned int serialNumber()
    {
        if (this->versionMajor() >= 2)
        {
            unsigned int serial;
            yk_get_serial(this->key_, 0, 0, &serial);
            return serial;
        }
        return 0;
    }

    std::string challengeResponse(const unsigned char *challenge, int length, int slot, bool hmac)
    {
        if (!this->key_)
        {
            return "";
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
};

struct LibraryManager
{
    LibraryManager()
    {
        LOG << "initializing yubikey library..." << endl;
        yk_init();
    }
    ~LibraryManager()
    {
        LOG << "releasing yubikey library..." << endl;
        yk_release();
    }
};

int main(int argc, char *argv[])
{
    ProgramOptions opts = ProgramOptions();
    try
    {
        opts.parse(argc, argv);
        LibraryManager yklibrary = LibraryManager();
        try
        {
            std::unique_ptr<YubiKey> key(new YubiKey());
            LOG << "version: " << key->version() << endl;
            LOG << "firmware: " << key->firmwareVersion() << endl;
            LOG << "serial number: " << key->serialNumber() << endl;
            LOG << "challenge: " << opts.challenge << endl;

            std::string unhexed;

            const unsigned char *chal;
            int len;
            if (opts.hexInput)
            {
                unhexed = boost::algorithm::unhex(opts.challenge);
                LOG << "length: " << unhexed.length() << endl;
                chal = reinterpret_cast<const unsigned char *>(unhexed.c_str());
                len = unhexed.length();
            }
            else
            {
                LOG << "length: " << opts.challenge.length() << endl;
                chal = reinterpret_cast<const unsigned char *>(opts.challenge.c_str());
                len = opts.challenge.length();
            }

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