for pid in $(ps -ef | grep -v grep | grep "nfs" | awk '{print $2}'); do kill -9 $pid; done
if test -d build; then
    fusermount -u build/disk
fi
rm -rf build
rm -f /tmp/out.log