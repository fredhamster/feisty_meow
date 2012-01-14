
# these metrics are how bogged down we are in to-do type items.

REPORT_FILE="$HOME/quartz/history_info_overload.txt"

# notes are individual files of tasks, usually, although some are combined.
note_depth=$(find ~/quartz/grunty_notes/ -type f -exec echo \"{}\" ';' | grep -v svn | wc -l)

# projects are slightly more productive, ongoing things that are very active.
project_depth=$(find ~/quartz/projects/ -type f -exec echo \"{}\" ';' | grep -v svn | wc -l)

# source examples need to be sucked into other places, other codebases.  they are not
# supposed to pile up here.
source_example_depth=$(find ~/quartz/example_source_code/ -type f -exec echo \"{}\" ';' | grep -v svn | wc -l)


#hmmm: need the counter of things in the html files back too.
#      those are doc'd in the notes about refactoring notes?

total_overload=$(($note_depth + $project_depth + $source_example_depth))
#hmmm: don't forget to add others.

report="\
\n\
We have studied your information overload and find that there are:\n\
  $note_depth note files\n\
  $project_depth project files\n\
  $source_example_depth source examples\n\
  -------\n\
  $total_overload total items\n\
Gathered On: $(date)\n\
##############\n\
"

echo -e "$report" | tee -a "$REPORT_FILE"

