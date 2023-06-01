fusermount -u build/disk
rm /tmp/out.log
sh scripts/test_basic.sh > /tmp/out.log || exit
diff scripts/test_basic.out /tmp/out.log || exit
echo ">>> test_basic PASSED"