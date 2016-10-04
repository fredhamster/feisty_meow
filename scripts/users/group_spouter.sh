sed -e 's/\([^:]*\):\([^:]*\):\([^:]*\):\([^:]*\):\([^:]*\):\([^:]*\):.*/sudo addgroup --gid \4 \1 ; sudo adduser \1 --uid \3 --gid \4/' < $CLOUD_BASE/configuration/users/fred_and_alts.passwd >$TMP/zz_spouted_groups.sh

Now running the $TMP/spouted_groups.sh script to create the groups:
bash $TMP/zz_spouted_groups.sh

