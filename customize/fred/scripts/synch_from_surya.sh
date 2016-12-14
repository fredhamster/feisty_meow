
# curie is the source for musix and also has limited space for walruses,
# so we check for that being our host here.
hostname -f 2>&1 | grep -i curie &>/dev/null
if [ $? -eq 0 ]; then
  # this name has curie in it, so we're probably on there.
  echo "Updating with special case for curie from surya"
  rsync -avz surya:/z/walrus/media/pictures/* /z/walrus/media/pictures/
  rsync -avz surya:/z/walrus/media/sounds/* /z/walrus/media/sounds/
else
  echo "Updating standard host from surya"
  rsync -avz surya:/z/walrus/* /z/walrus/
  rsync -avz surya:/z/musix/* /z/musix/
fi


