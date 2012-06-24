
# these metrics are how bogged down we are in to-do type items.

REPORT_FILE="$HOME/cloud/overload_history.txt"

# given a path, this will find how many items are under it, ignoring svn and git files, plus
# other patterns we happen to notice are not useful.
function calculate_depth()
{
  local dir="$1"; shift
  find "$dir" -type f -exec echo \"{}\" ';' |  grep -v "\.svn" | grep -v "\.git"| grep -v "\.basket" | grep -v "\.version" | grep -v "\.keep" | wc -l | tr -d ' '
}

# calculates the size in kilobytes of all the note files in a hierarchy.
# this is just a raw statistic for how much content all those notes make up.  since
# we have not separated out all the to-dos in some files (most notably the metaverse
# backlogs and to-do lists), it's good to also know what kind of girth the notes have.
function calculate_weight()
{
  local dir="$1"; shift
  find "$dir" -type f -exec echo \"{}\" ';' |  grep -v "\.svn" | grep -v "\.git"| grep -v "\.basket" | grep -v "\.version" | grep -v "\.keep" | xargs ls -al | awk '{ print $5 }' | paste -sd+ | bc
}

# produces a report line in our format.
function format_report_line()
{
  local depth="$1"; shift
  local weight="$1"; shift
  weight=$((weight / 1024))
  echo "  $depth\t${weight}kb\t$*\n"
}

# two parameters are needed: the directory to sum up and the label to use for it in the report.
# this will calculate the depth and weight for a hierarchy of notes, and then produce a
# line of reporting for those.
function analyze_hierarchy_and_report()
{
  local dir="$1"; shift
  local label="$1"; shift
  local depth=$(calculate_depth "$dir")
  total_overload=$(($depth + $total_overload))
  local weight=$(calculate_weight "$dir")
  total_weight=$(($total_weight + $weight))
  full_report+=$(format_report_line "$depth" "$weight" "$label")
}

# scans through items in the notes folder that begin with a pattern.
# each of those is treated as an aggregable portion of the report.
# first parameter is the title in the report, second and so on are
# a list of directory patterns to scan and aggregate.
function analyze_by_dir_patterns()
{
  local title="$1"; shift
  local hier_depth=0
  local hier_weight=0
  for i in $@; do
    temp_depth=$(calculate_depth $i)
    hier_depth=$(($hier_depth + $temp_depth))
    temp_weight=$(calculate_weight $i)
    hier_weight=$(($hier_weight + $temp_weight))
  done
  total_overload=$(($hier_depth + $total_overload))
  total_weight=$(($total_weight + $hier_weight))
  full_report+=$(format_report_line "$hier_depth" "$hier_weight" "$title")
}

##############

# reset these before we add anything...
total_overload=0
total_weight=0

# start out the report with a header.
full_report="\
\n\
Current information overload consists of:\n\
\n\
"

# notes are individual files of tasks, usually, although some are combined.
analyze_hierarchy_and_report ~/cloud/grunty_notes "grunty notes"

####
#hmmm: make an html todo scanning function from this.
# scan web documents for to-do lists.  individual items are marked with <li>.
# we do this one a bit differently since we have different criteria for html to-do items.
html_item_depth=$(find ~/cloud/grunty_notes/ -type f -iname "*.html" -exec grep "<li" "{}" ';' | wc -l | tr -d ' ')
total_overload=$(($html_item_depth + $total_overload))
html_item_weight=$(find ~/cloud/grunty_notes/ -type f -iname "*.html" -exec grep "<li" "{}" ';' | wc -c | tr -d ' ')
total_weight=$(($total_weight + $html_item_weight))
full_report+="$(format_report_line "$html_item_depth" "$html_item_weight" "to-do notes in html")"
####

# scan all the items declared as active projects.
analyze_by_dir_patterns "active items" ~/cloud/*active*

# scan across all appropriately named project or research folders that live in the "cloud".
analyze_by_dir_patterns "project files" ~/cloud/*project* ~/cloud/*research*

# source examples need to be sucked into other places, other codebases.  they are not
# supposed to pile up here.
analyze_hierarchy_and_report ~/cloud/example_source "source examples"

# also snag the files labelled as trivia, since they're still to-dos...
analyze_by_dir_patterns "trivial notes" ~/cloud/*trivia*

# and then count up the things that we think will be cleaned soon, but one thing we have learned
# unsorted files haven't been categorized yet.
analyze_hierarchy_and_report ~/cloud/unsorted "unsorted files"

# we now consider the backlog of things to read to be a relevant fact.  this is going to hose
# up our weight accounting considerably.
analyze_hierarchy_and_report ~/cloud/reading "reading list"

full_report+="\n\
  =====================================\n\
"
full_report+="$(format_report_line "$total_overload" "$total_weight" "Total Overload")"
full_report+="\n\
[gathered on $(date)]\n\n\
##############"

echo -e "$full_report" | tee -a "$REPORT_FILE"

