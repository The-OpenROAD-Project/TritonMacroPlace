mkdir -p /tritonmacroplace/build
cd /tritonmacroplace/build
cmake -DCMAKE_INSTALL_PREFIX=/tritonmacroplace/build -DBOOST_ROOT=/opt/rh/rh-mongodb32/root/usr ..
make