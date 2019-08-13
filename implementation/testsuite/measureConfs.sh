PROBLEM=$1

cd $PROBLEM
for g in game*
do
        cd $g
	echo -n "$g: "
	if [ "$PROBLEM" == "minepump" ]
	then
		echo 128
	else 
		if [ "$PROBLEM" == "elevator" ]
		then
			echo 32
		else
			echo "`head -n1 SVPG |sed 's/;//'|sed 's/confs\ //'|tr '-' '\n'|wc -l`-1"|bc
		fi
	fi
        cd ..
done
cd ..
