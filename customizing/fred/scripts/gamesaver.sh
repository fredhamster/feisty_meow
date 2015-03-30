
# hmmm: some better work could be done to have both names (wineprefix and storage dir) be iterable.
for i in oblivion fallout_new_vegas fallout_3 skyrim ; do
  if [ ! -d $HOME/spooling_saves_$i ]; then
    mkdir $HOME/spooling_saves_$i 
  fi
done
echo "======="
echo skyrim
cp -v -n ~/My\ Games/Skyrim/Saves/* ~/spooling_saves_skyrim
echo "======="
echo fallout new vegas
cp -v -n ~/My\ Games/FalloutNV/Saves/* ~/spooling_saves_fallout_new_vegas/
echo "======="
echo fallout 3
cp -v -n ~/My\ Games/Fallout3/Saves/Player1/* ~/spooling_saves_fallout_3
echo "======="
echo oblivion
cp -v -n ~/My\ Games/Oblivion/Saves/* ~/spooling_saves_oblivion/
echo "======="
