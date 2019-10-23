FILE=$1
working=`mktemp`
cp "$FILE" "$working"
while read -r r ; do
	color=`echo $r|cut -d'-' -f1`
	pattern=`echo $r|cut -d'-' -f2`
	sed -i "s/$color/$pattern/g" "$working"
done < replace
echo '<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" class="main-svg"  width="315" height="290" viewBox="55 85 370 365">' > "$FILE"
cat defs >> "$FILE"
cat "$working" |cut -d'>' --complement -f1 >> "$FILE"
rm -f "$working"
sed -i 's/sans-serif/arial/g' "$FILE"
sed -i "s/'Open Sans'/arial/g" "$FILE"
inkscape -z -e "$FILE.png" -w 315 -h 290 "$FILE"
