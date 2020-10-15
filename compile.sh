echo "COMPILING XRNETWORK"
echo "-------------------"

PWD=$(pwd)
echo "local path: $PWD"

cmake -S . -B build-debug -DCMAKE_BUID_TYPE=debug
