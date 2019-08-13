for p in minepump randomgames elevator
do
        ./measureConfs.sh $p  > `pwd`/data/measures_basic/${p}_confsize.data
done
