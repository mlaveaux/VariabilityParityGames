rm -f clean_*.csv
for f in *.csv
do
	cat $f |sed 's/game_phi//g'| sed 's/PGs//g' | sed 's/game_prop//g' |sed 's/gameFF//g' |sed 's/gameFC//g' |sed 's/gameBF//g' |sed 's/gameBC//g' |sed 's/game//g' > clean_$f	
done
