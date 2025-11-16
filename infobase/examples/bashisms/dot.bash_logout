
# on exit, clear the console for privacy.
if [ "$SHLVL" = 1 ]; then
  [ -x /usr/bin/clear_console ] && /usr/bin/clear_console -q
fi

# runs the nechung oracle program at exit from bash.
sep 79
$FEISTY_MEOW_GENERATED_STORE/runtime/binaries/nechung
echo
sleep 1

