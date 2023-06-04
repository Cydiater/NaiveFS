mkdir build
cd build
cmake .. 1> /dev/null || exit
make 1> /dev/null || exit

mkdir disk
./nfs -d disk > test_basic.log 2>&1 &
NFS_PID=$!
>&2 echo "nfs running in ${NFS_PID}"
sleep 1

cd disk
touch foo
echo hello > foo
ls -x
cat foo
cd ..

kill -9 $NFS_PID
sleep 1

./nfs -d disk > test_basic.log 2>&1 &
NFS_PID=$!
>&2 echo "nfs running in ${NFS_PID}"
sleep 1
if ps -p $NFS_PID > /dev/null; then
    echo "remount success"
else
    echo "remount failed"
    exit 1
fi

cd disk
cat foo
ls -x