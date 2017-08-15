使用方法：
服务端运行：./udp_server port
客户端运行：./udp_client serverIP serverPort

2个客户端分别处于内网，server处于外网
客户端启动时，server会告知此客户端的外网IP和端口
然后A客户端填入B的外网IP和端口
B客户端填入A的外网IP和端口

即可互相通信


注意！
千万不要让两个client共处与同一路由器下，路由器是不会转发发给自己的IP的包的。



