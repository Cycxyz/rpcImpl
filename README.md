# Assembling library

1. First copy the repository:

        git clone https://github.com/Cycxyz/rpcImpl

2. Go in the source folder, init and update submodules:

        git submodule init
        git submodule update

3. Assemble openssl. To do it you need:

    - Install Perl, add it to the PATH environment variable
    - Intall Nasm, add it to the PATH environment variable
    - Run Visual Studio developer command promt in openssl source directory
    - Run following commands:

            perl Configure
            nmake
            nmake test
            nmake install

4. After assembling openssl ensure in it's source directory exist libssl_static.lib and libssl_crypto.lib.

5. After that the rpcImpl library can be assembled. To assemble open command promt in source directory and run following commands:

        mkdir build
        cd build
        cmake ..
        cmake --build .

6. Done. Include files can be found in ***include*** folder, library file to link - ***build/CONFIGURATION/libSecureRPC.lib***