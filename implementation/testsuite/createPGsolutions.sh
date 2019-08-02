for pg in game_*
do
	echo $pg
	VPGSolver_bdd_single "$pg" R v f  | tail -n3|head -n1|awk -F':' '{print $2}'|sed 's/\ //'|sed 's/,$//'|tr ',' '\n'|sort -n > sol_${pg}_even
done
