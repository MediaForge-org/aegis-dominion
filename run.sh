#!/usr/bin/env bash
set -e
if [[ ! -x build/aegis_dominion ]]; then
  echo "Noch kein Build vorhanden — starte zuerst ./build.sh"
  exit 1
fi
cd build
./aegis_dominion
