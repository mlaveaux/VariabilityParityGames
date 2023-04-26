for i in `seq 1 7`;
do 
	echo "Do $i"
	mkdir -p PGs/prop${i};java -jar -Xss100M -Xmx6G ../../../testsuite/housekeeping.jar vpg FD elevator.features.aut simpleformulas/prop${i}.mcf PGs/prop${i}/
	echo "Solve $i"
	VPGSolver_bdd PGs/prop${i}/SVPG > PGs/prop${i}/log
done
