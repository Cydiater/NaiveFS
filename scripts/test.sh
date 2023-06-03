sh scripts/clear.sh
sh scripts/test_basic.sh > /tmp/out.log || exit
diff scripts/test_basic.out /tmp/out.log || exit
echo ">>> test_basic PASSED"