for alg in zlnk MBR
do
	cat randomgames_${alg}.data|grep gameC > randomgamesC_${alg}.data
	cat randomgames_${alg}.data|grep gameF > randomgamesF_${alg}.data
done
