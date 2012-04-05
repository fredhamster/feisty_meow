
# these metrics are how bogged down we are in to-do type items.

REPORT_FILE="$HOME/quartz/history_info_overload.txt"

# given a path, this will find how many items are under it, ignoring svn and git files.
function calculate_depth()
{
  local dir="$1"; shift
  find "$dir" -type f -exec echo \"{}\" ';' | grep -v "\.svn" | grep -v "\.git" | wc -l | tr -d ' '
}

##############

# notes are individual files of tasks, usually, although some are combined.
note_depth=$(calculate_depth ~/cloud/grunty_notes)

# unsorted files haven't been categorized yet.
unsorted_depth=$(calculate_depth ~/cloud/unsorted)

# source examples need to be sucked into other places, other codebases.  they are not
# supposed to pile up here.
source_example_depth=$(calculate_depth ~/quartz/example_source_code)

# the list files are web documents with to-do lists.  individual items are marked with <li>.
item_depth=$(find ~/cloud/grunty_notes/ -type f -iname "*.html" -exec grep "<li" "{}" ';' | wc -l | tr -d ' ')

# scan across all appropriately named folders in our folders that live in the "cloud".
cloud_depth=0
for i in ~/cloud/*project* ~/cloud/*research*; do
  temp_depth=$(calculate_depth $i)
  cloud_depth=$(($cloud_depth + $temp_depth))
done

##############

total_overload=$(($note_depth + $item_depth + $unsorted_depth + $source_example_depth + $cloud_depth))

report="\
\n\
Current information overload consists of:\n\
\n\
  $note_depth\tnote files\n\
  $item_depth\tto-do list items\n\
  $unsorted_depth\tunsorted files\n\
  $source_example_depth\tsource examples\n\
  $cloud_depth\tcloud notes\n\
  -------\n\
  $total_overload\ttotal items\n\
\n\
Gathered On: $(date)\n\
\n\
##############"

echo -e "$report" | tee -a "$REPORT_FILE"

