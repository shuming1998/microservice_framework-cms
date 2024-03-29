# microservice_framework: cms

**Windows/Linux 跨平台微服务框架 cms 源代码目录 src**

| 目录名称       | 功能      | 依赖关系(不含第三方库) |
| :------------- | --------- | :------------- |
| cmysql         | MySQL SDK            | 无 |
| cplatform      | 通信模块          | 无 |
| registerclient | 注册中心 客户端         | cplatform                                       |
| configclient   | 配置中心 客户端         | cplatform registerclient                        |
| configgui      | Windows 配置中心 GUI | cplatform                                       |
|                |                      |                                                 |
| registerserver | 注册中心             | cplatform                                       |
| clog           | 日志中心             | cmysql、register_client、cplatform              |
| configserver   | 配置中心             | cmysql、cplatform、registerclient               |
| cauth          | 鉴权中心             | cmysql、cplatform、registerclient、configclient |
| cmsgateway     | 网关                 | cmysql、cplatform、registerclient、configclient |



**项目依赖**

**[Windows 10](https://en.wikipedia.org/wiki/Windows_10)**

```
> Visual Studio 15 2017
> Qt 5.12
> OpenSSL 3.0
> zlib 1.2.11
> nasm 2.13.03
> libevent
> protobuf 3.8.0
> mysql 5.7.40
```



**Linux ([Ubuntu 20.04.5 LTS](https://releases.ubuntu.com/focal))**

0. 编译依赖工具

   ```shell
   $ sudo apt-get install gcc g++ perl make automake libtool
   ```

1. zlib-1.2.11 (https://github.com/madler/zlib)

   ```shell
   $ git clone -b 1.2.11 git@github.com:madler/zlib.git
   $ cd zlib
   $ make -j32
   ```

2. OpenSSL-3.0 (https://github.com/openssl/openssl)

   ```shell
   $ git clone -b openssl-3.0.0 git@github.com:openssl/openssl.git
   $ cd openssl
   $ ./config
   $ make -j32
   $ sudo make install
   ```

3. libevent (https://github.com/libevent/libevent)

   ```shell
   $ git clone git@github.com:libevent/libevent.git
   $ cd libevent
   $ ./configure
   $ make -j32
   $ sudo make install
   ```

4. MySQL 8.0

   ```shell
   $ sudo apt install net-tools
   $ sudo apt-get install mysql-server -y
   $ sudo apt install libmysqlclient-dev -y
   
   # 修改配置文件，设置不区分语句大小写
   $ sudo vim /etc/mysql/my.cnf
   [mysqld]
   lower_case_table_names=1
   
   # 更改初始密码
   $ mysql -u root -p
   Enter password: [初始密码]
   mysql> ALTER USER "root"@"localhost" IDENTIFIED BY "设置密码"
   mysql> exit
   
   # 配置任意 ip 可远程访问
   $ mysql -u root -p
   Enter password: [设置的密码]
   mysql> use mysql
   mysql> update user set host="%" where user="root";
   mysql> flush privileges;
   ```

   

**Linux 平台下编译和安装 cms**

```shell
$ git clone git@github.com:shuming1998/microservice_framework.git
$ cd microservice_framework
$ ./build_all.sh
```



**Linux 平台下运行/停止所有服务**

```shell
$ cd microservice_framework
$ start_all.sh
$ stop_all.sh
```

