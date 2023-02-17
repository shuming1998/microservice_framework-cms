stop_register_server
stop_config_server
stop_cauth
stop_clog
stop_cms_gateway
killall config_client
killall register_client
cd src
cd cmysql
make clean
make -j32
make install
cd ../
cd cplatform
make clean
make -j32
make install
cd ../
cd registerserver
make clean
make -j32
make install
cd ../
cd registerclient
make clean
make -j32
make install
cd ../
cd configserver
make clean
make -j32
make install
cd ../
cd configclient
make clean
make -j32
make install
cd ../
cd cmsgateway
make clean
make -j32
make install
cd ../
cd clog
make clean
make -j32
make install
cd ../
cd cauth
make clean
make -j32
make install
cd ../