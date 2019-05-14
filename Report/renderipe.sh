for i in *.ipe;do iperender -png -resolution 150 -transparent "$i" "`echo "$i"|sed 's/ipe$/png/'`";done
