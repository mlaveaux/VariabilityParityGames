PROBLEM=$1
CMD=$2
ARGS=$3
cd $PROBLEM
for g in game*
do
        cd $g
        echo -n "$g: "
	TIME=0
        for cc in sSVPG*
        do
		perl -pi -e 'chomp if eof' $cc
                perl -pi -e 'chomp if eof' $cc

                $CMD $cc R $ARGS > ../logs/${g}_${cc}.log
		ADD=`grep "Solving time" ../logs/${g}_${cc}.log|awk -F':' '{ print $2 }'|awk '{ print $1 }'`

                TIME=`echo $ADD + 0$TIME | bc`
        done
        cd ..
	echo $TIME
done
cd ..
