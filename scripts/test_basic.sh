cd build
rm -rf disk
mkdir disk
./nfs disk > test_basic.log 2>&1
sleep 1
cd disk
touch 123
ls -x
echo "Hello" > 123
touch 456
ls -x
cat 123 123 > 456
cat 456
ls -x
rm 123
ls -x