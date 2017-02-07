
# these metrics are how bogged down we are in to-do type items.

REPORT_FILE="$HOME/cloud/fred_stats/overload_history.txt"

# given a path, this will find how many items are under it, ignoring svn and git files, plus
# other patterns we happen to notice are not useful.
function calculate_count()
{
  local dir="$1"; shift
  local count=$(find "$dir" -type f -exec echo \"{}\" ';' 2>/dev/null |  grep -v "\.svn" | grep -v "\.git"| grep -v "\.basket" | grep -v "\.version" | grep -v "\.keep" | wc -l | tr -d ' ')
  if [ -z "$count" ]; then echo 0; else echo "$count"; fi
}

# calculates the size in kilobytes of all the note files in a hierarchy.
# this is just a raw statistic for how much content all those notes make up.  since
# we have not separated out all the to-dos in some files (most notably the metaverse
# backlogs and to-do lists), it's good to also know what kind of girth the notes have.
function calculate_weight()
{
  local dir="$1"; shift
  local weight=$(find "$dir" -type f -exec echo \"{}\" ';' 2>/dev/null | grep -v "\.svn" | grep -v "\.git"| grep -v "\.basket" | grep -v "\.version" | grep -v "\.keep" | xargs ls -al | awk '{ print $5 }' | paste -sd+ | bc 2>/dev/null)
  if [ -z "$weight" ]; then echo 0; else echo "$weight"; fi
}

# calculate_complexity gets a very simple metric of how many directory components are
# present at the target location and below.
function calculate_complexity()
{
  local dir="$1"; shift
  local complexity=$(find "$dir" -type d | wc -l)
  if [ -z "$complexity" ]; then echo 0; else echo "$complexity"; fi
}

# produces a report line in our format.
function format_report_line()
{
  local count="$1"; shift
  local weight="$1"; shift
  weight=$((weight / 1024))
  local complexity="$1"; shift
  echo "$count\t${complexity}\t\t${weight}\t\t$*\n"
}

# two parameters are needed: the directory to sum up and the label to use for it in the report.
# this will calculate the count and weight for a hierarchy of notes, and then produce a
# line of reporting for those.
function analyze_hierarchy_and_report()
{
  local dir="$1"; shift
  local label="$1"; shift
  local count=$(calculate_count "$dir")
  total_overload=$(($count + $total_overload))
  local weight=$(calculate_weight "$dir")
  total_weight=$(($total_weight + $weight))
  local complexity=$(calculate_complexity "$dir")
  total_complexity=$(($total_complexity + $complexity))
  full_report+=$(format_report_line "$count" "$weight" "$complexity" "$label")
}

# scans through items in the notes folder that begin with a pattern.
# each of those is treated as an aggregable portion of the report.
# first parameter is the title in the report, second and so on are
# a list of directory patterns to scan and aggregate.
function analyze_by_dir_patterns()
{
  local title="$1"; shift
  local hier_count=0
  local hier_weight=0
  local hier_complexity=0
  for folder in $@; do
    temp_count=$(calculate_count $folder)
    hier_count=$(($hier_count + $temp_count))
    temp_weight=$(calculate_weight $folder)
    hier_weight=$(($hier_weight + $temp_weight))
    temp_complexity=$(calculate_complexity $folder)
    hier_complexity=$(($hier_complexity + $temp_complexity))
  done
  total_overload=$(($hier_count + $total_overload))
  total_weight=$(($total_weight + $hier_weight))
  total_complexity=$(($total_complexity + $hier_complexity))
  full_report+=$(format_report_line "$hier_count" "$hier_weight" "$hier_complexity" "$title")
}

##############

# reset these before we add anything...
total_overload=0
total_weight=0

# start out the report with a header.
full_report="\
\n\
current information overload consists of:\n\
\n\
"
full_report+="count\tcomplexity\tweight (kb)\tcategory\n\
================================================================\n\
"

analyze_hierarchy_and_report ~/cloud/urgent "high priority (aieeee!)"

# notes are individual files of tasks, usually, although some are combined.
analyze_hierarchy_and_report ~/cloud/grunty_notes "grunty notes (externalities)"

# feisty notes are about feisty meow(r) concerns ltd codebase development.
analyze_hierarchy_and_report ~/cloud/feisty_notes "feisty meow notes (mondo coding)"

# home notes are a new top-level category; used to be under the grunty.
analyze_hierarchy_and_report ~/cloud/branch_road "hearth and home notes (branch road)"

##
#yeah, games don't count as technical debt or backlog.  that's nonsense.
#  the whole point of playing games is as a way to let off steam or escape from current
#  real world unpleasantness, so tying the game stuff into my huge lumbering backlog is
#  like saying lets inject toxins in our brain.  screw that.
##
## games and fun stuff.  not sure why these count as backlog, but whatever.
#analyze_hierarchy_and_report ~/cloud/gaming "games yo"

# some source code that needs to be sucked into other places, other codebases.  they are not
# supposed to pile up here.  but they have, so we track them.
#analyze_hierarchy_and_report ~/cloud/scavenging_source "source scavenging"

# and then count up the things that we think will be cleaned soon, but one thing we have learned
# unsorted files haven't been categorized yet.
analyze_hierarchy_and_report ~/cloud/disordered "unsorted files"

# we now consider the backlog of things to read to be a relevant fact.  this is going to hose
# up our weight accounting considerably.
analyze_hierarchy_and_report ~/cloud/reading "reading list (for a quiet afternoon)"

# scan all the items declared as active projects.
analyze_by_dir_patterns "active issues" ~/cloud/active*

# scan across all appropriately named project or research folders that live in the "cloud".
analyze_by_dir_patterns "running projects" ~/cloud/project* ~/cloud/research*

# snag any work related items for that category.
analyze_by_dir_patterns "jobby work tasks" ~/cloud/job* 

# scan all the trivial project folders.
analyze_by_dir_patterns "trivialities" ~/cloud/trivia*

full_report+="================================================================\n\
"
full_report+="$(format_report_line "$total_overload" "$total_weight" "$total_complexity" "total overload")"
full_report+="\n\
[gathered on $(date)]\n\n\
##############"

echo -e "$full_report" | tee -a "$REPORT_FILE"

