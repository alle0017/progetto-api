i=1

while [ $i -le 100 ]; do
      echo "executing test $i"

      p=$((1 + $RANDOM % 1000))
      c=$(( 1000 + $RANDOM ))
      a=$((1000 + $RANDOM))
      d=$((1000 + $RANDOM))
      l=$((1000 + $RANDOM))
      o=$((1000 + $RANDOM))
      z=$((1000 + $RANDOM))

      echo "period ${p}"
      echo "camion capacity ${c}"
      echo "recipes added ${a}"
      echo "recipes removed ${d}"
      echo "supplies ${l}"
      echo "orders ${o}"
      echo "wrong commands ${z}"

      ./test_gen_2024_macos -p ${p} -c ${c} -a ${a} -d ${d} -l ${l} -o ${o} -z ${z}  > test.txt

      echo "finding correct solution..."

      ./sol_2024_macos < ./test.txt > ./output.example.txt

      echo "finding answer..."

      time ./test < ./test.txt > ./output.txt

      diff --suppress-common-lines --side-by-side ./output.txt ./output.example.txt > diff.txt

      if [ $? -ne 0 ]; then
            echo "Differences found in file $i"
            break
      fi

      ((i++))
done

