
# these are my personal overrides.  --fred.

if [ -z "$USER_CUSTOMIZATIONS_LOADED" ]; then
  # if we don't see the customizations variable defined, this probably hasn't run yet.

  # stuff our special bins in front of the other bin paths.
  export PATH=/usr/local/fred/bin:$PATH

  # The nuage directory is a cloud-like repository of our personal data, managed as a git repo.
  export CLOUD_BASE=$HOME/nuage

  # The gruntose web site is expected to reside below, if it exists at all.
  export WEBBED_SITES=$HOME/web
  if [ "$(hostname)" = "hamstertronic" ]; then
    export WEBBED_SITES=/var/www
  fi

  # add a bunch of personal folders to the list for checkin & checkout.
  REPOSITORY_LIST=" nuage ebooks web ${REPOSITORY_LIST} "

  # adds our locally relevant archive folders into the list to be synched.
  MAJOR_ARCHIVE_SOURCES+="/z/archons /z/basement /z/imaginations /z/musix /z/toaster /z/walrus $HOME/brobdingnag"
  # our set of known source hierarchy folder names.
  SOURCECODE_HIERARCHY_LIST="codebarn extra_brain interbrane"

  # point to our local certificate for ssh usage.
  export SVN_SSH="ssh -i $HOME/.ssh/id_dsa_sourceforge"

  # Error and success noises for CLAM.
  export CLAM_ERROR_SOUND='/z/walrus/media/sounds/effects/bwaaang.wav /z/walrus/media/sounds/cartoons/doh4.wav'
  export CLAM_FINISH_SOUND='/z/walrus/media/sounds/cartoons/meepmeep.wav'

  # Setup for nethack adventure.
  export NETHACKOPTIONS="name:Manjusri-W,dogname:Fred,catname:Zonker"

  # mail setup for home machines.
#  export REPLYTO=fred@gruntose.com
#  export from="Fred T. Hamster <fred@gruntose.com>"

  # set our browser for seti and others that use the variable.
#  export BROWSER=/usr/bin/firefox

  # this hideous mess is necessitated by our not having found the source of
  # the "ls" color settings yet.  we override a few colors that look bad on
  # a dark background.
  export LS_COLORS='no=00:fi=00:di=01;37:ln=00;36:pi=40;33:so=01;35:do=01;35:bd=40;33;01:cd=40;33;01:or=41;33;01:ex=00;32:*.cmd=00;32:*.exe=01;32:*.com=01;32:*.bat=01;32:*.btm=01;32:*.dll=01;32:*.tar=00;31:*.tbz=00;31:*.tgz=00;35:*.rpm=00;33:*.deb=00;33:*.arj=00;31:*.taz=00;31:*.lzh=00;31:*.zip=00;35:*.zoo=00;31:*.z=00;31:*.Z=00;31:*.gz=00;35:*.bz2=00;31:*.tb2=00;31:*.tz2=00;31:*.tbz2=00;31:*.avi=01;35:*.bmp=01;35:*.fli=01;35:*.gif=01;35:*.jpg=01;35:*.jpeg=01;35:*.mng=01;35:*.mov=01;35:*.mpg=01;35:*.pcx=01;35:*.pbm=01;35:*.pgm=01;35:*.png=01;35:*.ppm=01;35:*.tga=01;35:*.tif=01;35:*.xbm=01;35:*.xpm=01;35:*.dl=01;35:*.gl=01;35:*.wmv=01;35:*.aiff=00;32:*.au=00;32:*.mid=00;32:*.mp3=00;32:*.ogg=00;32:*.voc=00;32:*.wav=00;32:'

  # options for the lame mp3 encoder.
  export LAMEOPT="--alt-preset extreme"

  # customization sentinel can be set now.
  export USER_CUSTOMIZATIONS_LOADED=true
fi


