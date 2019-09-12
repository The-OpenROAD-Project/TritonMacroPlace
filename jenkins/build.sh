docker build -f jenkins/Dockerfile.dev -t tritonmacroplace .
docker run -v $(pwd):/tritonmacroplace tritonmacroplace bash -c "./tritonmacroplace/jenkins/install.sh"