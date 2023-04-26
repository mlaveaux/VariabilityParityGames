game=$1
m=$2
p=`head -n1 $game |tail -n1|awk '{ print $2}'| sed 's/;//'`
step=`echo "($p/100)"|bc`
c=0
for n in `seq 0 $step $p`
do
	echo -n $c","
	VPGSolver_bdd_single "$game" R F P`cat "${game}_even" | tr '\n' ','` a$n |grep "Solving time"|awk '{ print $3}'
	c=$((c+1))
done
