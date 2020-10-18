echo "---------------------"
echo " COMPILING XRNETWORK "
echo "---------------------"

PWD=$(pwd)
echo "local path: $PWD"

BUILD_DEBUG="build-debug"
BUILD_RELEASE="build-release"
BIN_FOLDER="bin"

echo "debug folder: ${BUILD_DEBUG}"
echo "release folder: ${BUILD_RELEASE}"
echo "bin folder: ${BIN_FOLDER}"
echo ""

if [ -d "$BIN_FOLDER" ]; then rm -R $BIN_FOLDER; fi

select DTYPE in DEBUG RELEASE Cancel; do
    case $DTYPE in
	DEBUG)
	    echo "DEBUG SELECTED"
	    echo "deleting directory ${BUILD_DEBUG}"
	    if [ -d "$BUILD_DEBUG" ]; then rm -R $BUILD_DEBUG ; fi
	    cmake -S . -B ${BUILD_DEBUG} -DCMAKE_BUILD_TYPE=Debug -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=${BIN_FOLEDR}
	    cmake --build ${BUILD_DEBUG}
	    break
	    ;;
	RELEASE)
	    echo "RELESE SELECTED"
	    echo "deleting directory ${BUILD_RELEASE}"
	    if [ -d "$BUILD_RELEASE" ]; then rm -R $BUILD_RELEASE; fi
	    cmake -S . -B ${BUILD_RELEASE} -DCMAKE_BUILD_TYPE=Release
	    cmake --build ${BUILD_RELEASE}
	    break
	    ;;
	Cancel)
	    break
	    ;;
	*)
	    break
	    ;;
    esac
done

