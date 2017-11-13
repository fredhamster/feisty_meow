

#taking a comma (or any char) separated list and turning it into an array:

commaSeparatedList=tony,tiger,detroit,sugar,biscuits

IFS="," read -ra argArray <<< "$commaSeparatedList"

for ((i = 0; i < ${#argArray[@]}; i++)); do
  echo "arg $(($i + 1)): ${argArray[$i]}"
done

