PROBLEM=$1
ALGDIR=$2
name=`basename $ALGDIR`
cd $PROBLEM
mkdir measured$name
for g in game*
do
        cd $g
	echo $g
       	$ALGDIR/measure.sh SVPG > ../logs/${g}_measured$name.log
	cat ../logs/${g}_measured$name.log|grep Attracted|awk '{ print $2 }' > ../measured$name/$g
	ls sSVPG*|wc -l > ../measured$name/${g}_confs
        cd ..
done
cd ..
