#! /bin/sh
cd $HOME
sudo apt-get update && sudo apt-get install -y gcc-4.8 g++-4.8 libopencv-dev libboost1.55-all-dev cmake libcurlpp-dev uuid-dev git build-essential libcurl4-openssl-dev
mkdir develop
cd develop
mkdir emotions-app
cd emotions-app
wget https://download.affectiva.com/linux/affdex-cpp-sdk-3.2-20-ubuntu-xenial-xerus-64bit.tar.gz -O ./affdex-sdk.tar.gz
mkdir affdex-sdk
tar -xzvf affdex-sdk.tar.gz -C affdex-sdk
git clone https://github.com/Affectiva/cpp-sdk-samples
cd cpp-sdk-samples/ && mkdir build && cd build/
export AFFDEX_DATA_DIR=$HOME/develop/emotions-app/affdex-sdk/data
cmake -DBOOST_ROOT=/usr/ -DOpenCV_DIR=/usr/ -DAFFDEX_DIR=$HOME/develop/emotions-app/affdex-sdk ..
make
#emotions-app
mkdir -p $HOME/.emotions-app
echo 'URLBASE=192.168.0.1/emotions-app.php' > $HOME/.emotions-app/emotions-app.conf && echo 'enjoy :)'
