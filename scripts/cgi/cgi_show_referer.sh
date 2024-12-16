#!/usr/bin/env bash
echo "Content-type: text/plain"
echo ""
echo ""

export query=$(echo $QUERY_STRING | sed -e "s/^[^=][^=]*=\(.*\)$/\1/")
export referer=$(echo $HTTP_REFERER | sed -e "s/^\(.*\/\).*$/\1/")
echo "--------"
echo "query is $query"
echo ""
echo "referer is $referer"
echo "--------"

echo ""
echo ""
echo "env is\n\n $(env) "


