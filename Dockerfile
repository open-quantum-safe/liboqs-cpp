FROM ubuntu:latest

# Install dependencies
RUN apt-get -y update && \
    apt-get install -y build-essential git cmake libssl-dev

# Get liboqs
RUN git clone --depth 1 --branch main https://github.com/open-quantum-safe/liboqs

# Install liboqs
RUN cmake -S liboqs -B liboqs/build -DBUILD_SHARED_LIBS=ON && \
    cmake --build liboqs/build --parallel 4 && \
    cmake --build liboqs/build --target install

# Enable a normal user
RUN useradd -m -c "Open Quantum Safe" oqs
USER oqs
WORKDIR /home/oqs

# Get liboqs-cpp
RUN git clone --depth 1 --branch main https://github.com/open-quantum-safe/liboqs-cpp.git

# Configure and install liboqs-cpp
RUN cmake -S liboqs-cpp -B liboqs-cpp/build
USER root
RUN cmake --build liboqs-cpp/build --target install
USER oqs

# Build liboqs-cpp examples and unit tests
RUN cmake --build liboqs-cpp/build --target examples --target unit_tests --parallel 4
