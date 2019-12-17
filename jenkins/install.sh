mkdir -p /tritonmacroplace/build
cd /tritonmacroplace/build
cmake -DCMAKE_INSTALL_PREFIX=/tritonmacroplace/build -DBUILD_PYTHON=0 -DBUILD_TCL=0 ..
make
