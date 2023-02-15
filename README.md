# Microservice_Framework

### Environment (root)

#### MooseFS:

```shell
$ apt install build-essential libpcap-dev zlib1g-dev libfuse-dev pkg-config
$ git clone git@github.com:moosefs/moosefs.git
$ cd moosefs
$ ./linux_build.sh

# 0. If mfsmount install failed for fuse version to old: build new version fuse from source code: 
$ apt install ninja-build meson
$ git clone git@github.com:libfuse/libfuse.git
$ cd libfuse && mkdir build && cd build
$ meson ..
$ cd ../
$ python3 -m pytest test/
$ ninja install

# 1. configure and run mfsmaster
$ groupadd mfs
$ useradd -g mfs mfs
$ cd /etc/mfs
$ cp mfsmaster.cfg.sample mfsmaster.cfg
$ cp mfsexports.cfg.sample mfsexports.cfg
$ cd /var/lib/mfs
$ cp metadata.mfs.empty metadata.mfs
$ chown mfs:mfs /var/lib/mfs
$ mfsmaster

###############################################
Configure Multiple Chunkservers:      
Install mfs in other server  		    
Repeat the  2. step		          
###############################################

# 2. configure and run mfschunkserver
$ echo "[outer ip]  mfsmaster" | grep /etc/hosts
$ mkdir /mnt/hd1
$ mkdir /mnt/hd2
$ chown mfs:mfs /mnt/hd1
$ chown mfs:mfs /mnt/hd2
$ chmod 770 /mnt/hd1
$ chmod 770 /mnt/hd2
$ cd /etc/mfs
$ cp mfschunkserver.cfg.sample mfschunkserver.cfg
$ cp mfshdd.cfg.sample mfshdd.cfg
$ vim mfshdd.cfg
/mnt/hd1
/mnt/hd2
$ mfschunkserver

# 3. begin to use mfs
$ mkdir /mnt/mfs
$ mfsmount -H mfsmaster /mnt/mfs
```



