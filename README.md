This is a simple example demonstrating how to use the yubikey library.

Example Usage:

    $> ./bin/ykchalresp --verbose --slot=2 challenge_text

    initializing yubikey library...
    opened key: xxxxxxx
    version: 4.2.7
    firmware: 1
    serial number: xxxxxxx
    challenge: challenge_text
    length: 14
    243FB4EF5C8031452C349D742003B411DA25150C
    closing key...
    releasing yubikey library...

# Dependencies

cmake, gcc7, boost, libyubikey, libykpers

## On Ubuntu:

`apt install build-essential cmake libboost1.62-all-dev libyubikey-dev libykpers-1-dev`

# Build Instructions

1. Clone the repository
2. `$ mkdir build`
3. `$ cd build`
4. `$ cmake ..`
5. `$ make`
