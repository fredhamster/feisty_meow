
# overrides for the web user, www-data.

if [ -z "$USER_CUSTOMIZATIONS_LOADED" ]; then

  # if we don't see the customizations variable defined, this probably hasn't run yet.

  ##############
 
  # The gruntose web site is expected to reside below, if it exists at all.
  export WEBBED_SITES="$FEISTY_MEOW_PERSONAL_HOME/web"

  REPOSITORY_LIST_TO_PULL+="$WEBBED_SITES"
  REPOSITORY_LIST_TO_COMMIT+="$WEBBED_SITES"

  ##############

  # customization sentinel can be set now.
  export USER_CUSTOMIZATIONS_LOADED=true
fi


