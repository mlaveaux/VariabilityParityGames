./time.sh $1 /home/sjef/AfstudeerRepo/implementation/testsuite/algs/ZlnkBDD/ > $1/logs/time_bdd.log
./attrtime.sh $1 /home/sjef/AfstudeerRepo/implementation/testsuite/algs/ZlnkBDD/ > $1/logs/attrtime_bdd.log

./time_single.sh $1 /home/sjef/AfstudeerRepo/implementation/testsuite/algs/ZlnkBDD/ > $1/logs/time_single.log
./attrtime_single.sh $1 /home/sjef/AfstudeerRepo/implementation/testsuite/algs/ZlnkBDD/ > $1/logs/attrtime_single.log

./time.sh $1 /home/sjef/AfstudeerRepo/implementation/testsuite/algs/ZlnkExplicit/ > $1/logs/time_explicit.log
./time.sh $1 /home/sjef/AfstudeerRepo/implementation/testsuite/algs/ZlnkExplicit/ > $1/logs/time_explicit.log

