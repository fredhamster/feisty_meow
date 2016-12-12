
# this script is meant to be run on curie with our super alpha prime source of music plugged in.

#hmmm: add the goodness around these like the nice updater.

if [[ ! ( $(hostname) =~ .*curie.* ) ]]; then
  echo this script is only designed to run on curie with the
  echo fred music prime external disc plugged in.
  exit 1
fi

# hmmm: these look very similar for musix and basement.  function please!
rsync -av /media/fred/fredmusicprime/musix/* /z/musix/
rsync -av /media/fred/fredmusicprime/basement/* /z/basement/
rsync -avz /z/musix/* surya:/z/musix/ 
rsync -avz /z/basement/* surya:/z/basement/ 
rsync -avz /z/musix/* wildmutt:/z/musix/ 
rsync -avz /z/basement/* wildmutt:/z/basement/ 
rsync -avz /z/musix/* euphrosyne:/z/musix/ 
rsync -avz /z/basement/* euphrosyne:/z/basement/ 


