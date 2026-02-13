
# overrides for the web user, www-data.

if [ -z "$USER_CUSTOMIZATIONS_LOADED" ]; then

  # if we don't see the customizations variable defined, this probably hasn't run yet.

  ##############

  REPOSITORY_LIST_TO_PULL+="$FEISTY_MEOW_PERSONAL_HOME/web "
  REPOSITORY_LIST_TO_COMMIT+="$FEISTY_MEOW_PERSONAL_HOME/web "

  ##############

  # customization sentinel can be set now.
  export USER_CUSTOMIZATIONS_LOADED=true
fi


