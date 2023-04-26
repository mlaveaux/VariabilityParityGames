for f in *.data
do
	cat $f | sed 's/:\ /,/' > csvs/$f.csv
done
