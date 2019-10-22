FILE=$1
working=`mktemp`
cp "$FILE" "$working"
while read -r r ; do
	color=`echo $r|cut -d'-' -f1`
	pattern=`echo $r|cut -d'-' -f2`
	sed -i "s/$color/$pattern/g" "$working"
done < replace
echo '<svg class="main-svg" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" width="1000" height="500" viewBox="0 0 1000 500">' > "$FILE"
cat defs >> "$FILE"
cat "$working" |cut -d'>' --complement -f1 >> "$FILE"
rm -f "$working"
sed -i 's/sans-serif/arial/g' "$FILE"
sed -i "s/'Open Sans'/arial/g" "$FILE"
inkscape -z -e "$FILE.png" -w 1000 -h 500 "$FILE"
