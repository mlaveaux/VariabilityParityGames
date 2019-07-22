ALGDIR=$1
for g in game*
do
	cd $g
	echo "Start $g"
	for cc in sSVPG*
	do
		c=`echo $cc|sed 's/sSVPG//'`
		$ALGDIR/run_single_local.sh SVPG $c > ../logs/${g}_${c}.log
		$ALGDIR/output2even.sh ../logs/${g}_${c}.log |tr -d '\n' > ../logs/${g}_${c}.zero
		if ! diff ../logs/${g}_${c}.zero sol_sSVPG${c}_zero > /dev/null
		then
			echo "Inconsistency found in $g with configuration $c"
		fi
	done
	cd ..
done
