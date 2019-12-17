set -e
docker run -v $(pwd):/TritonMP tritonmp bash -c "/TritonMP/test/regression fast"
docker run -v $(pwd):/TritonMP tritonmp bash -c "cat /TritonMP/test/results/diffs"
