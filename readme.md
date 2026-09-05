1. Add this to ~/.ssh/config

Host cs425-*
  HostName fa26-%h.cs.illinois.edu
  User weilunt2
  IdentityFile ~/.ssh/cs425
  StrictHostKeyChecking accept-new

2. ssh-keygen -t ed25519 -f ~/.ssh/cs425 -C "cs425 vms"

3. 輸入十台的密碼
for i in $(seq 4201 4210); do
  ssh-copy-id -i ~/.ssh/cs425.pub cs425-$i
done

./remote.sh deploy: git clone下我們的專案＋make
./remote.sh start: 執行 ./bins/server -i $id
./remote.sh stop: Remote server for all machines