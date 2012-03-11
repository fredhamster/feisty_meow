
# these metrics are how bogged down we are in to-do type items.

REPORT_FILE="$HOME/quartz/history_info_overload.txt"

# notes are individual files of tasks, usually, although some are combined.
note_depth=$(find ~/quartz/grunty_notes/ -type f -exec echo \"{}\" ';' | grep -v "\.svn" | grep -v "\.git" | wc -l)

# the list files are web documents with to-do lists.  individual items are marked with <li>.
item_depth=$(find ~/quartz/grunty_notes/ -type f -iname "*.html" -exec grep "<li" "{}" ';' | wc -l)

# projects are slightly more productive, ongoing things that are very active.
project_depth=$(find ~/quartz/projects/ -type f -exec echo \"{}\" ';' | grep -v "\.svn" | grep -v "\.git" | wc -l)

# source examples need to be sucked into other places, other codebases.  they are not
# supposed to pile up here.
source_example_depth=$(find ~/quartz/example_source_code/ -type f -exec echo \"{}\" ';' | grep -v "\.svn" | grep -v "\.git" | wc -l)

total_overload=$(($note_depth + $item_depth + $project_depth + $source_example_depth))

report="\
We have studied your information overload and find that there are:\n\
  $note_depth note files\n\
  $item_depth to-do list items\n\
  $project_depth project files\n\
  $source_example_depth source examples\n\
  -------\n\
  $total_overload total items\n\
Gathered On: $(date)\n\
\n\
##############\n\
"

echo -e "$report" | tee -a "$REPORT_FILE"

