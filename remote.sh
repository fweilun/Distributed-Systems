#!/usr/bin/env bash
# usage: ./cluster.sh {deploy|start|stop|status|collect}
set -u

REPO=https://github.com/fweilun/Distributed-Systems.git
DIR=Distributed-Systems
HOSTS=$(seq 4201 4210)
SSH="ssh -n -o BatchMode=yes -o ConnectTimeout=5"

deploy() {
  for h in $HOSTS; do
    (
      $SSH cs425-$h "
        if [ -d $DIR/.git ]; then
          cd $DIR && git fetch origin && git reset --hard origin/main
        else
          git clone $REPO $DIR && cd $DIR
        fi
        make
      " && echo \"[ok]   $h\" || echo \"[FAIL] $h\"
    ) &
  done
  wait
}

start() {
  id=1
  for h in $HOSTS; do
    (
      $SSH cs425-$h "{ cd $DIR && ./bins/server -i $id; } >/dev/null 2>&1 </dev/null &" \
        && echo "[start] $h id=$id" || echo "[FAIL] $h"
    ) &
    id=$((id+1))
  done
  wait
}


stop() {
  for h in $HOSTS; do
    $SSH cs425-$h "pkill -f 'bins/server' || true"
    echo "[stop] $h"
  done
}e

case "${1:-}" in
  deploy)  deploy ;;
  start)   start ;;
  stop)    stop ;;
  *) echo "usage: $0 {deploy|start|stop}"; exit 1 ;;
esac