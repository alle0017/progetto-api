#!/bin/bash
gcc -std=gnu11 -o heap heap.h
gcc -std=gnu11 -o hash hash.h
gcc -std=gnu11 -o store store.h
gcc -std=gnu11 -o functions functions.h
gcc -std=gnu11 -o queue queue.h
gcc -std=gnu11 -o recipes recipes.h
gcc -std=gnu11 -o struct struct.h
gcc -std=gnu11 -o commands commands.h
gcc -std=gnu11 -fsanitize=address -o main main.c

time ./main < ../test_cases_pubblici/open10.txt > output

start_line=595000
end_line=596000

while [ $end_line -le 601000 ]; do
    # Extract the specific lines and number them
    sed -n "${start_line},${end_line}p" andre.txt | nl -v ${start_line} > a.txt
    sed -n "${start_line},${end_line}p" output | nl -v ${start_line} > b.txt

    # Run diff command
    diff --suppress-common-lines --side-by-side -b -w a.txt b.txt > diff.txt

    # Check if diff command found differences
    if [ $? -ne 0 ]; then
        echo "Differences found between lines $start_line and $end_line"
        break
    fi

    # Increment the line numbers
    start_line=$((start_line + 1000))
    end_line=$((end_line + 1000))
done

