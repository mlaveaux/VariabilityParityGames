game=$1
conf=$2
p=`head -n2 $game/SVPG |tail -n1|awk '{ print $2}'| sed 's/;//'`
for n in `seq 0 15 $p`
do
	echo -n $n","
	VPGSolver_bdd_single $game/SVPG c$conf F P`cat $game/sol_sSVPG${conf}_even | tr '\n' ','` a$n |grep "Solving time"|awk '{ print $3}'
done
