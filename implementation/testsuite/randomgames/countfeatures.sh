for f in game*;do echo -n "$f: ";head -n1 $f/SVPG|sed 's/confs //'|sed 's/;//'|wc -L;done
