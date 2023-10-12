# SimpleHttpServer
Implement a simple HTTP service

在命令行内执行下列命令（linux系统）
1. git clone https://github.com/Chitoy/SimpleHttpServer.git
2. cd build
3. cmake ..
4. make
5. ./HttpServer
6. 项目依赖curl和json库
7. json不需要额外安装，项目自身带有，curl库需要额外安装
   这样你的http服务器就已经运行了，可以通过浏览器访问你的服务器
注意：确保linux服务器的防火墙允许外界访问端口80
