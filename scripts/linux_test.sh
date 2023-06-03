rm -rf ../build
mkdir ../build
cd ../build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j4
cd ..
sh scripts/test_large.sh
