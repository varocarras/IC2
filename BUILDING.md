## Requirements
 - OpenSSL
 - Libdatachannel;
 - VS++

## Instructions
cmake -G "Visual Studio 16 2019" -A x64 -B "build64"
cmake --build build64 --config Release

## Static Build
 - In order to statically link all the dependencies, first statically build them individually, then add them to the corresponding CMakelist.txt by setting setOpenSSL(True) and LibDataChannel-Static
