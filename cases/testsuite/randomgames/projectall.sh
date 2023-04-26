for f in game*
do
	echo $f
	VPGSolver_bdd $f/SVPG Q$f/
done
