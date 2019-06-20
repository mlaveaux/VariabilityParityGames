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


for i in 2 3 5;do echo "Do $i";time java -jar -Xss100M -Xmx6G ../../../testsuite/housekeeping.jar vpg /home/sjef/AfstudeerRepo/implementation/WorkingExamples/Elevator/sjef/FD /home/sjef/AfstudeerRepo/implementation/WorkingExamples/Elevator/sjef/elevator.original.features.aut /home/sjef/AfstudeerRepo/implementation/WorkingExamples/Elevator/sjef/simpleformulas/prop${i}.mcf /home/sjef/AfstudeerRepo/implementation/WorkingExamples/Elevator/sjef/PGs/prop${i}/ > /tmp/log$i;done
