rm -f clean_*.csv
for f in *.csv
do
	cat $f | sed 's/prop3a/prop3/g' | sed 's/prop3b/prop4/g' |sed 's/game_scaleconfs_//g' |sed 's/game_phi//g'| sed 's/PGs//g' | sed 's/game_prop//g' |sed 's/gameFF//g' |sed 's/gameFC//g' |sed 's/gameBF//g' |sed 's/gameBC//g' |sed 's/game//g' |sort -n > clean_$f
done
for f in clean_randomscalegames*.csv
do
	sed -i "s/a,/.0,/g" "$f"
        sed -i "s/b,/.25,/g" "$f"
        sed -i "s/c,/.5,/g" "$f"
        sed -i "s/d,/.75,/g" "$f"
done

