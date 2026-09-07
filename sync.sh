for h in $(seq 4201 4210); do
  rsync -az --exclude bins --exclude .git ./ cs425-$h:Distributed-Systems/ &
done
wait