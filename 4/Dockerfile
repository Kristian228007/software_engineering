FROM ubuntu:24.04 AS builder

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    git \
    ca-certificates \
    libssl-dev \
    pkg-config \
    libpq-dev \
    postgresql-server-dev-all \
    && rm -rf /var/lib/apt/lists/*

# Build POCO from source with MongoDB and JWT support
ARG POCO_VERSION=1.15.0
WORKDIR /build
RUN git clone --depth 1 --branch poco-${POCO_VERSION}-release https://github.com/pocoproject/poco.git

WORKDIR /build/poco
RUN mkdir cmake-build && cd cmake-build \
    && cmake .. -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DPOCO_ENABLE_MONGODB=ON \
    -DPOCO_ENABLE_DATA_POSTGRESQL=ON \
    -DPOCO_ENABLE_JWT=ON \
    && cmake --build . --target install -j$(nproc)

# Build application
WORKDIR /build/app
COPY CMakeLists.txt ./
COPY src ./src/

RUN mkdir build && cd build \
    && cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/usr/local \
    && cmake --build . -j$(nproc)

# Runner stage
FROM ubuntu:24.04 AS runner

RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    libpq-dev \
    && rm -rf /var/lib/apt/lists/*

# Copy POCO shared libraries from builder
COPY --from=builder /usr/local/lib/libPoco*.so* /usr/local/lib/

# Copy application binary
COPY --from=builder /build/app/build/server /usr/local/bin/

COPY --from=builder /build/app/src/swagger.yaml /usr/local/bin/swagger.yaml

ENV LD_LIBRARY_PATH=/usr/local/lib

EXPOSE 8080

CMD ["server"]