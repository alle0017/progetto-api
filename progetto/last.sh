gcc -fsanitize=address -DEVAL -Wall -Werror -std=gnu11 -O2 -pipe -o test test.c -lm

i=1

while [ $i -le 11 ]; do
      echo "executing test $i"
      time ./test < ./test_cases_pubblici/open${i}.txt > output.txt
      # Run diff command
      diff --suppress-common-lines --side-by-side output.txt ./test_cases_pubblici/open${i}.output.txt > diff.txt

      # Check if diff command found differences
      if [ $? -ne 0 ]; then
            echo "Differences found in file $i"
            break
      fi

      # Increment the line numbers
      ((i++))
done