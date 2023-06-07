sh scripts/clear.sh
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make 
mkdir disk
./nfs disk
touch disk/1
cd ..
fio -filename=build/disk/1 -direct=1 -iodepth 1 -thread -rw=randrw -rwmixread=70 -ioengine=psync -bs=16k -size=128M -numjobs=4 -runtime=100 -group_reporting -name=mytest -ioscheduler=noop