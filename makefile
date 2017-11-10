include variables.def

PROJECT = feisty_meow_sources
FIRST_TARGETS += start_make
BUILD_BEFORE = scripts \
  infobase walrus \
  nucleus octopi graphiq \
  kona \
  documentation huffware hypermedia \
  production \
  experiments 
LAST_TARGETS += end_make

include rules.def

start_make: show_date.start

end_make: show_date.end

