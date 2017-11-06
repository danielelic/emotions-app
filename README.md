# emotions-app
emotions-app project

Build and run: `emotions-app`
------------
```sh
mkdir build
cd build
cmake -DBOOST_ROOT=/usr/ -DOpenCV_DIR=/usr/ -DAFFDEX_DIR=$HOME/develop/emotions-app/affdex-sdk DCURL_LIBRARY=/usr/lib -DCURL_INCLUDE_DIR=/usr/include
./emotions-app/emotions-app -d ../../affdex-sdk/data/
```