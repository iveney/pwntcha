#!/bin/sh

rm -f index.html && echo > index.html
echo '<html><body bgcolor="#aaddff"><h1>CAPTCHA samples</h1><ul>' >> index.html

find -maxdepth 2 -name '*00*' | while read i; do
  echo "$i"
  DIRNAME="`echo $i | sed 's,/[^/]*$,,'`"
  EXTENSION="`echo $i | sed 's/.*00//'`"
  INDEX="$DIRNAME.html"
  rm -f "$INDEX" && echo > "$INDEX"
  echo '<html><body bgcolor="#aaddff"><table>' >> "$INDEX"
  N=1
  for x in `seq -w 0 99`; do
    echo '<tr>' >> "$INDEX"
    echo '<td><h1><tt>#'"$x"'</tt></h1></td>' >> "$INDEX"
    echo '<td><img src="'"$DIRNAME/$DIRNAME$x$EXTENSION"'" /></td>' >> "$INDEX"
    TEXT="[untranslated]"
    if [ -r "$DIRNAME/control.txt" ]; then
        TEXT="`sed -ne ${N}p "$DIRNAME/control.txt"`"
        N=$(($N + 1))
    fi
    echo '<td><h1><tt>'"$TEXT"'</tt></h1></td>' >> "$INDEX"
    echo '</tr>' >> "$INDEX"
  done
  echo '</table></body></html>' >> "$INDEX"

  echo '<li><a href="'"$INDEX"'">'"$DIRNAME"'</a></li>' >> index.html
done

echo '</ul></body></html>' >> index.html

