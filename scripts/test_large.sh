cd build
rm -rf disk
mkdir disk
./nfs disk > test_basic.log 2>&1
cd ../scripts
./test_random 50000000
diff ans ../build/disk/1