
我的問題是：
我有五台機器同時傳送5MB到server

1. server要使用connection還是recv接收就好

2. server怎麼知道這個瘋包是誰傳送的（可能message要加一個屬性）

3. server accept

然後我不知道在幹嘛



最後一個對 bind() 的額外小提醒：在你不願意呼叫 bind() 時。若你正使用 connect() 連線到遠端的機器，你可以不用管 local port 是多少（以 telnet 為例，你只管遠端的 port 就好），你可以單純地呼叫 connect()，它會檢查 socket 是否尚未綁定（unbound），並在有需要的時候自動將 socket bind() 到一個尚未使用的 local port。
：我猜這是對client端可以使用的




int select(int numfds, fd_set *readfds, fd_set *writefds,
           fd_set *exceptfds, struct timeval *timeout);

fd_set master;


FD_ZERO(&master);

// 避開這個錯誤訊息："address already in use"
setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

FD_SET(listener, &master)


select(fdmax+1, &read_fds, NULL, NULL, NULL)
在fdmax以下的都會被詢問，有點像同時accept多個連線，一有就會有回傳值。

socket:
配置一個空間，並且在df_table上佔一個編號

bind: 向kernel註冊對應到socket_id的addr, port等，讓kernel知道要把封包傳到哪裡。
connect 會自動幫你bind好


fcntl(fd, cmd, ..) 純屬設定file屬性。

flags = fcntl(fd, F_GETFL) socket剛socket()建立時的屬性，預設是O_RDWR
non-blocking 的好處是

client端：
    connect parallelize
    recv non blocking
    send 看buffer還有多少空間


read_fds = []
for
    create sockets
    set sockets to non-blocking
    add socket to read_fds

while 1
    select()
    for i in 1..max_fds
        if i not in read_fds contiune

        recv(i) & print
    
    


for each machine:
    create socket, connect, send request, shutdown(SHUT_WR)
    FD_SET(fd, &master)
remaining = N

while remaining > 0:
    read_fds = master          // 複製
    tv = 剩餘時間               // 每輪重設
    select(fdmax+1, &read_fds, NULL, NULL, &tv)
    if 逾時: 剩下的標記失敗, break
    for each fd in master:
        if !FD_ISSET(fd, &read_fds): continue
        n = recv(fd, ...)
        n > 0 → 拆行、印、算 count
        n == 0 → close, FD_CLR, remaining--