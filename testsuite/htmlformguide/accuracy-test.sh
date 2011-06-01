#!/bin/bash

#i=0
#for file in *.jpg
#do 
#	istr=`printf "%.3d" $i`
#	mv $file `echo $file | sed "s/captcha\(.*\).jpg/\htmlformguide_$istr.jpg/"`
#	let i=$i+1
#done

if (($# < 2))
then
	echo "Usage: accuracy-test.sh <pwntcha binary> <pwntcha share>"
	exit
fi

numTests=0
for file in *.jpg
do 
	$1 -s $2 $file >> results
	let numTests=$numTests+1
done

diff control results | grep ">" >> errors

i=0
while read line
do
	let i=$i+1
done <errors

rm results >> /dev/null
rm errors >> /dev/null

let accuracy=$numTests-$i
echo "Total accuracy: $accuracy/$numTests"

