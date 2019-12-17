docker build --target=base-dependencies -t tritonmp .
docker run -v $(pwd):/TritonMP tritonmp bash -c "./TritonMP/jenkins/install.sh"