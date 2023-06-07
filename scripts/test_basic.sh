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

mkdir dir1
mkdir dir2
cd dir1
touch foo
rm -r ../dir2
cp ../../../scripts/64MB.file foo
cd ..
diff ../../scripts/64MB.file dir1/foo
ls -x
rm -r dir1

cp ../../scripts/4096B.file foo
diff foo ../../scripts/4096B.file
echo hello > foo
cat foo

cp ../../scripts/64MB.file foo
diff foo ../../scripts/64MB.file
mv foo bar
diff bar ../../scripts/64MB.file
mkdir dir
mv bar dir/bar
diff dir/bar ../../scripts/64MB.file
ls -x
ls -x dir

if ps -p $NFS_PID > /dev/null; then
   kill -9 $NFS_PID
else
   echo "nfs exited"
   exit 1
fi