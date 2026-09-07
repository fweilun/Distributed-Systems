#!/usr/bin/env bash
# usage: ./cluster.sh {deploy|start|stop|status|collect}
set -u

REPO=https://github.com/fweilun/Distributed-Systems.git
DIR=Distributed-Systems
HOSTS=($(seq 4201 4210))
DOMAIN="fa26-cs425"
SSH="ssh -n -o BatchMode=yes -o ConnectTimeout=5"
SCP="scp -o BatchMode=yes -o ConnectTimeout=5"


deploy() {
  for h in "${HOSTS[@]}"; do
    (
      $SSH fa26-cs425-${h}.cs.illinois.edu "
        if [ -d $DIR/.git ]; then
          cd $DIR && git fetch origin main && git reset --hard origin/main
        else
          git clone $REPO $DIR && cd $DIR
        fi
        make
      " && echo \"[ok]   $h\" || echo \"[FAIL] $h\"
    ) &
  done
  wait
}

# used for unit test, delete if need
push_logs() {
  id=1
  for h in "${HOSTS[@]}"; do
    local_file="./logs/machine.${id}.log"
    if [ ! -f "$local_file" ]; then
      echo "[ERROR] $local_file not found locally!"
      id=$((id+1))
      continue
    fi

    (
      target="fa26-cs425-${h}.cs.illinois.edu"
      # 明確呼叫 ssh，不要帶 -n
      ssh -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null -o BatchMode=yes -o ConnectTimeout=5 \
        "$target" "mkdir -p ~/$DIR/logs && cat > ~/$DIR/logs/machine.${id}.log" < "$local_file" \
        && echo "[push] $h machine.${id}.log" || echo "[FAIL] $h push log"
    ) &
    id=$((id+1))
  done
  wait
}

start() {
  if [ -z "${1:-}" ]; then
  id=1
  for h in "${HOSTS[@]}"; do
    (
      $SSH fa26-cs425-${h}.cs.illinois.edu "{ cd $DIR && nohup ./bins/server -i $id; } >/dev/null 2>&1 </dev/null &" \
        && echo "[start] $h id=$id" || echo "[FAIL] $h"
    ) &
    id=$((id+1))
  done
  else
  target=$1
  $SSH fa26-cs425-${HOSTS[$((target - 1))]}.cs.illinois.edu "{ cd $DIR && nohup ./bins/server -i $target; } >/dev/null 2>&1 </dev/null &" \
        && echo "[start] ${HOSTS[$((target - 1))]} id=$target" || echo "[FAIL] $target"
  fi
  wait
}


stop() {
  if [ -z "${1:-}" ]; then
  id=1
  for h in "${HOSTS[@]}"; do
    $SSH fa26-cs425-${h}.cs.illinois.edu "pkill -f 'bins/server' || true"
    echo "[stop] $h"
  done
  else
  target=$1
  $SSH fa26-cs425-${HOSTS[$((target - 1))]}.cs.illinois.edu "pkill -f 'bins/server' || true"
  fi
}

case "${1:-}" in
  deploy)    deploy ;;
  start)     start "${2:-}" ;;
  stop)      stop "${2:-}";;
  push_logs) push_logs;;
  *) echo "usage: $0 {deploy|start|stop|push_logs}"; exit 1 ;;
esac