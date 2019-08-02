for f in *.csv
do
	cat $f |sed 's/game_phi//g'| sed 's/PGs//g' | sed 's/game_prop//g' |sed 's/gameF//g' |sed 's/gameC//g' |sed 's/game//g' > clean_$f	
done
