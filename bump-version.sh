#!/bin/sh

if [ "$#" -ne 1 ]; then
  echo Usage: $0 tag_name
  exit 1
fi
TAG=$1
sed -i "s+^VERSION = .*+VERSION = $TAG+" config.mk
git add config.mk
git commit -m "Bumped version $TAG"
