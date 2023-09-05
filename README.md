# FTP Proxy Protocol Proxy

## 项目简介

FTP Proxy Protocol Proxy是一个用于代理FTP（File Transfer Protocol）协议流量的开源代理服务器。它允许用户在FTP客户端和FTP服务器之间建立连接，同时提供了一些增强的功能和安全性选项。

该代理服务器的主要特点包括：

- **安全传输：** 支持FTP数据传输的加密，使用TLS/SSL协议来保护数据的机密性。
- **访问控制：** 允许管理员配置访问控制规则，以控制哪些用户和IP地址可以访问FTP服务器。
- **日志记录：** 记录FTP流量和代理操作的详细日志，以便审计和故障排除。
- **多平台支持：** 该代理服务器可以在Windows、Linux和macOS等多个平台上运行。
- **高性能：** 使用多线程和异步编程来实现高并发和高性能的FTP代理。

## 快速开始

### 依赖项


### 安装和运行

1. 克隆或下载本仓库。

```shell
git clone https://github.com/shangfenglu/net_world.git

1.cd ftp-proxy/out/build/x64-debug/ftp_proxy
  ./ftp_proxy your_server_ip
2.cd ftp_proxy
  mkdir build
  cd build
  cmake ..
  make
  ./ftp_proxy your_server_ip

