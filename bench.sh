p=$(pwd)
mkdir -p build && cd build && cmake .. && make
cd $p
cd tests && python bench.py && mv bench.json $p
cd $p
