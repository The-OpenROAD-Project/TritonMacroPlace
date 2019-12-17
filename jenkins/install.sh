mkdir -p /TritonMP/build
cd /TritonMP/build
cmake -DCMAKE_INSTALL_PREFIX=/TritonMP/build -DBUILD_PYTHON=0 -DBUILD_TCL=0 ..
make -j 4
