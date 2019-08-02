for file in randomgames total_randomgames
do
	cat $file.data|grep gameC > ${file}C.data
	cat $file.data|grep gameF > ${file}F.data
done


