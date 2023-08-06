#!/bin/bash

cc="m68k-amigaos-gcc"
search_dir="/media/Daten/ADev/Tests/C/Grafik"
outfile="$search_dir/.vscode/compile_commands.json"

echo >$outfile "[";
for entry in "$search_dir"/*.c
do
    echo >>$outfile "{";
    echo >>$outfile "\"directory\": \"$search_dir\",";
    echo >>$outfile "\"file\": \"$entry\","
    echo >>$outfile "\"command\": \"$cc $entry\""
    echo >>$outfile "},";
done
echo >>$outfile "{}";
echo >>$outfile "]";
