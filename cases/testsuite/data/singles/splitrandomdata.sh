for e in FF FC BC
do
	rm ${e}_randomgames*.data
done
for f in *randomgames*.data
do
	FILE=${f%.data}
	for e in FF FC BC
	do
		cat $FILE.data|grep game$e > ${e}_${FILE}.data
	done
done
