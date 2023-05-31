sh scripts/test_basic.sh > /tmp/out.log
diff scripts/test_basic.out /tmp/out.log
echo ">>> test_basic PASSED"