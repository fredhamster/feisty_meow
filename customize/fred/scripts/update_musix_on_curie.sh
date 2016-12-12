
# this script is meant to be run on curie with our super alpha prime source of music plugged in.

#hmmm: add the goodness around these like the nice updater.

if [[ ! ( $(hostname) =~ .*curie.* ) ]]; then
  echo this script is only designed to run on curie with the
  echo fred music prime external disc plugged in.
  exit 1
fi

rsync -av /media/fred/fredmusicprime/musix/* /z/musix/
rsync -avz /z/musix/* surya:/z/musix/ 
rsync -avz /z/musix/* wildmutt:/z/musix/ 
rsync -avz /z/musix/* euphrosyne:/z/musix/ 


