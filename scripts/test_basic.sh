cd build
rm -rf disk
mkdir disk
./nfs disk > test_basic.log 2>&1
sleep 1
cd disk
ls -x
touch 123
ls -x
ls -x > 123
touch 456
ls -x
cat 123 123 > 456
cat 456 456 > 123
cat 123 123 > 456
cat 456 456 > 123
cat 123 123 > 456
cat 456 456 > 123
cat 123 123 > 456
cat 456 456 > 123
cat 123 123 > 456
cat 456 456 > 123
cat 123 123 > 456
cat 456 456 > 123
rm 123
ls -x
kill -9 ${NFS_PID}
