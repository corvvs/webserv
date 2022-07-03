#!/bin/bash
CYAN="\033[36m"
RESET="\033[0m"

CONFIG='../conf'

# chmod 000 `find $CONFIG/ -name "*permission*"`
# find $CONFIG/invalid -name "*.conf" | sort > tempfile
# echo "=====INVALID CONFIG TEST====="
# while read line
# do
#     echo -en $CYAN"[CASE] "
#     filename=(${line//\// })
#     echo ${filename[3]}
#     echo -en $RESET

#     ./a.out $line
# done < tempfile

find $CONFIG/valid -name "*.conf" | sort > tempfile
echo "=====VALID CONFIG TEST====="
while read line
do
    echo -en $CYAN"[CASE] "
    filename=(${line//\// })
    echo ${filename[3]}
    echo -en $RESET

    ./a.out $line
done < tempfile
rm tempfile
