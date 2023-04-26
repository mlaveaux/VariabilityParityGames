for p in minepump elevator randomgames
do 
	cd $p
	for g in game*
	do 
		echo $p/$g
		cd $g
		createSUVPG sSVPG*> SUVPG
		cd ..
	done
	cd ..
done
