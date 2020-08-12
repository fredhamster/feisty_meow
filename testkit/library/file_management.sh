
# these are the pieces that we'll use to assemble mostly random files.
RANDOM_CHUNK_FILES=($TEST_TEMP/random.0 $TEST_TEMP/random.1 $TEST_TEMP/random.2 $TEST_TEMP/random.3 $TEST_TEMP/random.4)

# largest chunk of random data we'll actually generate at a time, in each chunk file.
export MAX_CHUNK_FILE=65536

# returns the file size for the first argument.
function getFileSize()
{
  local file="$1"; shift
  if isMacOSX; then
    stat -f%z "$file"
  else
    stat --printf="%s" "$file"
  fi
}

# outputs the number of seconds since the epoch.
function getTimeStamp()
{
  date +%s
}

# makes sure the chunk files are all generated.
function prepareRandomChunks()
{
  local i
  for ((i = 0; i < ${#RANDOM_CHUNK_FILES[@]}; i++)); do
    # make the chunk files if they don't exist.
    local currfile="${RANDOM_CHUNK_FILES[$i]}"
    if [ ! -f "$currfile" ]; then
      local filesize=$MAX_CHUNK_FILE
      # pick a value to add or subtract from the constant sized chunk, so we won't always be
      # using files at the same boundaries or with a power of 2 size.
      local moddy=$(( ($(echo $RANDOM) % 128) - 64 ))
      ((filesize -= $moddy))
#echo creating chunk file $currfile of size $filesize
      dd if=/dev/urandom of=$currfile bs=1 count=$filesize &>/dev/null
      assertEquals "creating random chunk file $currfile" 0 $?
    fi
  done
}

# creates a somewhat random file for testing.  this will be assembled out of
# our chunks of random files, so is not truly random, but we've found that the
# random number generator is a HUGE drag on our testing speed.  this is still
# pretty random data.  the first argument is the file name and the second is
# the desired file size.
function createRandomFile()
{
  local file="$1"; shift
  local size="$1"; shift

  prepareRandomChunks

  local stampBefore=$(getTimeStamp)

  # truncate any existing stuff.
  echo -n >"$file"

  while [ $(getFileSize "$file") -lt $size ]; do
    which_chunker=$(expr $(echo $RANDOM) % ${#RANDOM_CHUNK_FILES[@]})
#echo choosing chunk file $which_chunker
    cat "${RANDOM_CHUNK_FILES[$which_chunker]}" >>"$file"
  done

#echo file size after random chunkings is $(getFileSize "$file")

  local fsizenow="$(getFileSize "$file")"
#echo size now is $fsizenow and desired is $size
  if [ $fsizenow -gt $size ]; then
#echo trying to truncate file
    truncate -s $size "$file"
  fi
#echo file size after truncate is $(getFileSize "$file") and expected size is $size

  local stampAfter=$(getTimeStamp)
  local secs=$(($stampAfter - $stampBefore))
  if [ $secs -le 0 ]; then
    # even though it claims it took zero time, we know better, but we also don't want to
    # divide by zero, so it loses its credit for being very fast here.
    secs=1
  fi
  local kbs=$(( $size / $secs / 1024))
  
  local fsizenow="$(getFileSize "$file")"
  assertEquals "Creating random file of $size bytes at ${kbs} kbps in: $file" $size $fsizenow
}

