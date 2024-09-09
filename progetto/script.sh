#!/bin/bash

# Compile the C program with AddressSanitizer
gcc -std=gnu11 -fsanitize=address -o test test.c

# Run the program and redirect output to output.txt
time ./test < test.txt > output.txt

# Add line numbers to output.txt and output.example.txt
nl output.txt > output_numbered.txt
nl output.example.txt > output.example_numbered.txt

# Run the diff command with the numbered outputs
diff --suppress-common-lines --side-by-side output_numbered.txt output.example_numbered.txt > diff.txt

# Check the diff result and output the appropriate message
if [ $? -ne 0 ]; then
    echo "Differences found"
else
    echo "all fine"
fi
