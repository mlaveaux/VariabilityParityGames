cd randomverification
./verify.sh /home/sjef/AfstudeerRepo/implementation/testsuite/algs/ZlnkExplicit/ > logs/verify_explicit.log
./verify_single.sh /home/sjef/AfstudeerRepo/implementation/testsuite/algs/ZlnkBDD/ > logs/verify_single.log
./verify.sh /home/sjef/AfstudeerRepo/implementation/testsuite/algs/ZlnkBDD/ > logs/verify_bdd.log
cd ..
./time.sh minepump /home/sjef/AfstudeerRepo/implementation/testsuite/algs/ZlnkBDD/ > minepump/logs/time_bdd.log
./time.sh minepump /home/sjef/AfstudeerRepo/implementation/testsuite/algs/ZlnkExplicit/ > minepump/logs/time_explicit.log

./attrtime.sh minepump /home/sjef/AfstudeerRepo/implementation/testsuite/algs/ZlnkBDD/ > minepump/logs/attrtime_bdd.log
./attrtime.sh minepump /home/sjef/AfstudeerRepo/implementation/testsuite/algs/ZlnkExplicit/ > minepump/logs/attrtime_explicit.log

./time_single.sh minepump /home/sjef/AfstudeerRepo/implementation/testsuite/algs/ZlnkBDD/ > minepump/logs/time_single.log
./attrtime_single.sh minepump /home/sjef/AfstudeerRepo/implementation/testsuite/algs/ZlnkBDD/ > minepump/logs/attrtime_single.log

