
# hmmm: some better work could be done to have both names (wineprefix and storage dir) be iterable.
for i in oblivion fallout_new_vegas fallout_3 skyrim ; do
  if [ ! -d $HOME/spooling_saves/$i ]; then
    mkdir -p $HOME/spooling_saves/$i 
  fi
done
echo "======="
echo skyrim
cp -v -n ~/wine_goods/My\ Games/Skyrim/Saves/* ~/spooling_saves/skyrim
echo "======="
echo fallout new vegas
cp -v -n ~/wine_goods/My\ Games/FalloutNV/Saves/* ~/spooling_saves/fallout_new_vegas/
echo "======="
echo fallout 3
cp -v -n ~/wine_goods/My\ Games/Fallout3/Saves/Player1/* ~/spooling_saves/fallout_3
echo "======="
echo oblivion
cp -v -n ~/wine_goods/My\ Games/Oblivion/Saves/* ~/spooling_saves/oblivion/
echo "======="
