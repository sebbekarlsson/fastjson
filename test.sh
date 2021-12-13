p=$(pwd)
mkdir -p build && cd build && cmake .. && make && cd tests && ./fjson_test
cd "$p" || return
