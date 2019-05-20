
# these metrics are how bogged down we are in to-do type items.

# logged historical file where we append our latest report.
REPORT_FILE="$CLOUD_BASE/stats/overload_history.txt"

#hmmm: check path validity?

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

  # initial values are all zero.
  local count=0
  local weight=0
  local complexity=0

  if [ -d "$dir" ]; then
    count=$(calculate_count "$dir")
    total_overload=$(($count + $total_overload))
    weight=$(calculate_weight "$dir")
    total_weight=$(($total_weight + $weight))
    complexity=$(calculate_complexity "$dir")
    total_complexity=$(($total_complexity + $complexity))
  fi
  full_report+=$(format_report_line "$count" "$weight" "$complexity" "$label")
}

# scans through items in the notes folder that begin with a pattern.
# each of those is treated as an aggregatable portion of the report.
# first parameter is the title in the report, second and so on are
# a list of directory patterns to scan and aggregate.
function analyze_by_dir_patterns()
{
  local title="$1"; shift
  local hier_count=0
  local hier_weight=0
  local hier_complexity=0
  for folder in $@; do
    if [ -d "$folder" ]; then
      temp_count=$(calculate_count $folder)
      hier_count=$(($hier_count + $temp_count))
      temp_weight=$(calculate_weight $folder)
      hier_weight=$(($hier_weight + $temp_weight))
      temp_complexity=$(calculate_complexity $folder)
      hier_complexity=$(($hier_complexity + $temp_complexity))
    fi
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

#hmmm: don't fail if the hierarchy doesn't exist.

# high priority stuff would be called urgent.
analyze_hierarchy_and_report $CLOUD_BASE/urgent "high priority (aieeee!)"

# notes are individual files of tasks, usually, although some are combined.
analyze_hierarchy_and_report $CLOUD_BASE/grunty* "grunty (external facing) notes"

# web site development tasks.
analyze_hierarchy_and_report $CLOUD_BASE/webular "web design (ideas and tasks)"

# feisty notes are about feisty meow(r) concerns ltd codebase development.
analyze_hierarchy_and_report $CLOUD_BASE/feisty_notes "feisty meow notes (mondo coding)"

# metaverse notes are about our ongoing simulator development and LSL scripting.
analyze_hierarchy_and_report $CLOUD_BASE/metaverse "metaverse in cyberspace design and scripting"

# home notes are a new top-level category; used to be under the grunty.
analyze_hierarchy_and_report $CLOUD_BASE/branch_road "hearth and home notes (branch road)"

# and then count up the things that we think will be cleaned soon, but one thing we have learned
# unsorted files haven't been categorized yet.
analyze_hierarchy_and_report $CLOUD_BASE/disordered "disordered and maybe deranged files"

# we now consider the backlog of things to read to be a relevant fact.  this is going to hose
# up our weight accounting considerably.
analyze_hierarchy_and_report $CLOUD_BASE/reading "reading list (for a quiet afternoon)"

####

# vocation is a prefix for anything i consider part of my life's work.
analyze_by_dir_patterns "life's work and other oddities" $CLOUD_BASE/vocation*

# scan all the items declared as active projects.
analyze_by_dir_patterns "active issues" $CLOUD_BASE/active*

# rub alongside all the travel notes to see if any have interesting burrs.
analyze_by_dir_patterns "travel plans" $CLOUD_BASE/walkabout*

# scan across all appropriately named project or research folders.
analyze_by_dir_patterns "running projects" $CLOUD_BASE/project* $CLOUD_BASE/research*

# look for our mad scientist style efforts.
analyze_by_dir_patterns "lab experiments" $CLOUD_BASE/experiment*

# snag any work related items for that category.
analyze_by_dir_patterns "jobby work tasks" $CLOUD_BASE/job* 

# scan all the trivial project folders.
analyze_by_dir_patterns "trivialities and back burner items" $CLOUD_BASE/trivia* $CLOUD_BASE/backburn*

full_report+="================================================================\n\
"
full_report+="$(format_report_line "$total_overload" "$total_weight" "$total_complexity" "total overload")"
full_report+="\n\
[gathered on $(date)]\n\n\
##############"

echo -e "$full_report" | tee -a "$REPORT_FILE"

