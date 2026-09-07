for h in $(seq 4201 4210); do
  rsync -az --exclude bins --exclude .git ./ fa26-cs425-$h.cs.illinois.edu:Distributed-Systems/ &
done
wait
