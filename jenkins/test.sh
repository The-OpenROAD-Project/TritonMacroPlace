set -e
docker run -v $(pwd):/tritonmacroplace tritonmacroplace bash -c "/tritonmacroplace/test/regression fast"
docker run -v $(pwd):/tritonmacroplace tritonmacroplace bash -c "cat /tritonmacroplace/test/results/diffs"
