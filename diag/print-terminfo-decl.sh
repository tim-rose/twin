#!/bin/sh
noarg_capabilities="init home cup cud cuu cuf cub cnorm civis cvvis"
printf '/*\n * '
tput longname
printf '\n */\n'
adjust_ctl()
{
    filter='s/ /y/g;'
    filter='
s||\\001|g;
s||\\002|g;
s||\\003|g;
s||\\004|g;
s||\\005|g;
s||\\006|g;
s||\\007|g;
s||\\b|g;
s|	|\\t|g;
#s|
#|\\n|g;
s||\\013|g;
s||\\f|g;
s||\\r|g;
s||\\016|g;
s||\\017|g;
s||\\020|g;
s||\\021|g;
s||\\022|g;
s||\\023|g;
s||\\024|g;
s||\\025|g;
s||\\026|g;
s||\\027|g;
s||\\030|g;
s||\\031|g;
s||\\032|g;
s||\\033|g;
s||\\034|g;
s||\\035|g;
s||\\036|g;
s||\\037|g'

    sed -e "$filter"
 }

for cap in ${*:-$noarg_capabilities}; do
    printf 'char %s[]="%s";\n' "$cap" "$(tput $cap)"
done | adjust_ctl
