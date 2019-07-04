echo "Time with bdd"
./time.sh $1 /home/sjef/AfstudeerRepo/implementation/testsuite/algs/ZlnkBDD/ > $1/logs/time_bdd.log
echo "Attr with bdd"
./attrtime.sh $1 /home/sjef/AfstudeerRepo/implementation/testsuite/algs/ZlnkBDD/ > $1/logs/attrtime_bdd.log

echo "Time with single"
./time_single.sh $1 /home/sjef/AfstudeerRepo/implementation/testsuite/algs/ZlnkBDD/ > $1/logs/time_single.log
echo "Attr with single"
./attrtime_single.sh $1 /home/sjef/AfstudeerRepo/implementation/testsuite/algs/ZlnkBDD/ > $1/logs/attrtime_single.log

echo "Time with explicit"
./time.sh $1 /home/sjef/AfstudeerRepo/implementation/testsuite/algs/ZlnkExplicit/ > $1/logs/time_explicit.log
echo "Attr with explicit"
./attrtime.sh $1 /home/sjef/AfstudeerRepo/implementation/testsuite/algs/ZlnkExplicit/ > $1/logs/attrtime_explicit.log

echo "Time with Oink"
./time_single.sh $1 /home/sjef/AfstudeerRepo/implementation/testsuite/algs/Oink/ > $1/logs/time_oink.log
