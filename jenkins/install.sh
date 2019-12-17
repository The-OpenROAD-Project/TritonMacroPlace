mkdir -p /tritonmacroplace/build
cd /tritonmacroplace/build
cmake -DCMAKE_INSTALL_PREFIX=/tritonmacroplace/build -DBUILD_PYTHON=0 -DBUILD_TCL=0 -DCMAKE_CXX_COMPILER=/opt/rh/devtoolset-6/root/usr/bin/g++ -DCMAKE_C_COMPILER=/opt/rh/devtoolset-6/root/usr/bin/gcc ..
make
