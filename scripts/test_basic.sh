cd build
rm -rf disk
mkdir disk
./nfs -d disk > test_basic.log 2>&1 &
NFS_PID=$!
echo "running nfs at ${NFS_PID}" >&2
cd disk
ls -x
touch 123
ls -x
touch 456
ls -x
rm 123
ls -x
kill -9 ${NFS_PID}