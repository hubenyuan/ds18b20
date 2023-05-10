# ds18b20
##介绍
本项目主要是树莓派上通过一线协议(1-Wire)连接DS18B20，采用网络socket进行客户端和服务器的连接。
客户端负责实现定时上报数据的功能，服务器则用来接受数据并把数据永久保存到数据库中。

##客户端代码框架

![屏幕截图 2023-05-10 181505](https://github.com/hubenyuan/ds18b20/assets/130223262/f9bf6ed3-a032-4350-a2a6-2aebf34a5857)

##服务器代码框架

![image](https://github.com/hubenyuan/ds18b20/assets/130223262/8ebbec7c-8d0e-4591-9358-c4703a513367)
