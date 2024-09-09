#gcc -std=gnu11 -o heap heap.h
#gcc -std=gnu11 -o hash hash.h
#gcc -std=gnu11 -o store store.h
#gcc -std=gnu11 -o functions functions.h
#gcc -std=gnu11 -o queue queue.h
#gcc -std=gnu11 -o recipes recipes.h
#gcc -std=gnu11 -o struct struct.h
#gcc -std=gnu11 -o commands commands.h
#gcc -std=gnu11 -fsanitize=address -o main main.c

#time ./main < ../test_cases_pubblici/open8.txt > output

#diff --suppress-common-lines --side-by-side output ../test_cases_pubblici/open8.output.txt > diff.txt

#time ./bundle2 < test_cases_pubblici/open7.txt > output

gcc -Wall -Werror -std=gnu11 -o bundle bundle.c

i=1

while [ $i -le 11 ]; do
      echo "executing test $i"
      time ./main < ../test_cases_pubblici/open${i}.txt > output
      # Run diff command
      diff --suppress-common-lines --side-by-side output ../test_cases_pubblici/open${i}.output.txt > diff.txt

      # Check if diff command found differences
      if [ $? -ne 0 ]; then
            echo "Differences found in file $i"
            break
      fi

      # Increment the line numbers
      ((i++))
done