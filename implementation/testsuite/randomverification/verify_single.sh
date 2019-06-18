ALGDIR=$1
for g in game*
do
	cd $g
	echo "Start $g"
	for cc in sSVPG*
	do
		c=`echo $cc|sed 's/sSVPG//'`
		$ALGDIR/run_single_fulloutput.sh SVPG $c > ../logs/${g}_${c}.log
		$ALGDIR/output2even.sh ../logs/${g}_${c}.log > ../logs/${g}_${c}.even
		if ! diff ../logs/${g}_${c}.even sol_sSVPG${c}_even > /dev/null
		then
			echo "Inconsistency found in $g with configuration $c"
		fi
	done
	cd ..
done
