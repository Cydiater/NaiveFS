rm -rf build
mkdir build
cd build
cmake -DSMALL_SEGMENT=ON ..
make -j4

mkdir disk
./nfs disk > test_basic.log 2>&1
sleep 1

cd disk
touch foo
cat ../../scripts/4096B.file > foo
diff foo ../../scripts/4096B.file