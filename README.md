# py-libva
libva python binding

## how build & run

install dependencies
```bash
sudo pip3 install pybind11 
sudo pip3 install json2html
# note: if need proxy, add "--proxy https://xxx.xxx.xxx:port" in cmd line
```

set environment variables
```bash
python3 gen_env.sh
source env.sh
```

build python binding so
```bash
cd py-libva
mkdir build
cd build
cmake ../src
make
```

run caps query example
```bash
cd py-libva/src
python3 caps.py
```