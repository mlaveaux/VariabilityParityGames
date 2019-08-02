for g in game*
do
	cd $g
	for pg in sSVPG*
	do
		echo $g/$pg
		pgsolver -global recursive $pg |tail -n8|head -n1|sed 's/{//'|sed 's/}//'|sed 's/ //g'|tr ',' '\n'|sort -n > sol_${pg}_even
	done
	cd ..
done
