

for dir in "${@}"; do
  /bin/ls -1fR "$dir" | grep -v "^$" | grep -v "^\.$" | grep -v "^\.\.$" | grep -v "$dir\/.*:$" | wc
done


