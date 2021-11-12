#!/bin/bash

echo "Run ASAN"
echo "meson . build -Dtests=true -Dloaders=\"svg, tvg, png, jpg\" -Dsavers=\"tvg\" -Dbindings=\"capi\" -Db_sanitize=address,undefined --errorlogs"

cd ./build/test

./tvgUnitTests > memcheck_asan.txt 2>&1

PAYLOAD_MEMCHECK=`cat memcheck_asan.txt`
COMMENTS_URL=$(cat $GITHUB_EVENT_PATH | jq -r .pull_request.comments_url)

echo $COMMENTS_URL
echo "MEMCHECK errors:"
echo $PAYLOAD_MEMCHECK

if [[ $PAYLOAD_MEMCHECK == *"A"* ]]; then
  OUTPUT+=$'\n**MEMCHECK(AddressSanitizer) RESULT**:\n'
  OUTPUT+=$'\n`meson . build -Dtests=true -Dloaders="svg, tvg, png, jpg" -Dsavers="tvg" -Dbindings="capi" -Db_sanitize=address,undefined`\n'
  OUTPUT+=$'\n```\n'
  OUTPUT+="$PAYLOAD_MEMCHECK"
  OUTPUT+=$'\n```\n' 
fi

PAYLOAD=$(echo '{}' | jq --arg body "$OUTPUT" '.body = $body')

curl -s -S -H "Authorization: token $GITHUB_TOKEN" --header "Content-Type: application/vnd.github.VERSION.text+json" --data "$PAYLOAD" "$COMMENTS_URL"
