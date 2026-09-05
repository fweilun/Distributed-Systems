while read -r ip; do
  ssh -o ConnectTimeout=2 "$ip" "killall -9 server 2>/dev/null" &
done < machines.txt
wait
sleep 0.5
