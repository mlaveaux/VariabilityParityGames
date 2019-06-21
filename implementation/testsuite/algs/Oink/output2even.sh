cat $1|tail -n3|head -n1|awk -F':' '{ print $2 }'|sed 's/ //'|sed 's/,$//'|tr ',' '\n'|sort -n
