PROBLEM=$1
ALGDIR=$2
name=`basename $ALGDIR`
cd $PROBLEM
mkdir measuredBasis
for g in game*
do
        cd $g
	ls sSVPG*|wc -l > ../measured$name/${g}_confs
	echo -n "$g: "
        cd ..
done
cd ..
