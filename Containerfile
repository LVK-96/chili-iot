FROM ubuntu:24.04

LABEL description="Build environment for Chili IoT Project"

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    wget \
    unzip \
    xz-utils \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Tool versions
ARG ARM_GCC_VERSION=14.3.rel1
ARG NINJA_VERSION=1.13.2
ARG CMAKE_VERSION=4.2.1
ARG GCC_VERSION=14
ARG PYTHON_VERSION=3.14
ARG LLVM_VERSION=20

# ARM Toolchain
RUN wget -q https://developer.arm.com/-/media/Files/downloads/gnu/${ARM_GCC_VERSION}/binrel/arm-gnu-toolchain-${ARM_GCC_VERSION}-x86_64-arm-none-eabi.tar.xz \
    && tar -xf arm-gnu-toolchain-${ARM_GCC_VERSION}-x86_64-arm-none-eabi.tar.xz -C /opt \
    && mv /opt/arm-gnu-toolchain-${ARM_GCC_VERSION}-x86_64-arm-none-eabi /opt/gcc-arm \
    && rm arm-gnu-toolchain-${ARM_GCC_VERSION}-x86_64-arm-none-eabi.tar.xz
ENV PATH="/opt/gcc-arm/bin:$PATH"

# Ninja
RUN wget -q https://github.com/ninja-build/ninja/releases/download/v${NINJA_VERSION}/ninja-linux.zip \
    && unzip ninja-linux.zip -d /usr/local/bin \
    && rm ninja-linux.zip
ENV CMAKE_GENERATOR="Ninja"

# CMake
RUN wget -q https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-linux-x86_64.sh \
    && chmod +x cmake-${CMAKE_VERSION}-linux-x86_64.sh \
    && mkdir -p /opt/cmake \
    && ./cmake-${CMAKE_VERSION}-linux-x86_64.sh --skip-license --prefix=/opt/cmake --exclude-subdir \
    && rm cmake-${CMAKE_VERSION}-linux-x86_64.sh
ENV PATH="/opt/cmake/bin:$PATH"

RUN apt-get update && apt-get install -y \
    curl \
    git \
    vim \
    build-essential \
    software-properties-common \
    lsb-release \
    gnupg \
    && rm -rf /var/lib/apt/lists/*

# Add additional repositories
# Ubuntu Toolchain - newer version of GCC
RUN add-apt-repository -y ppa:ubuntu-toolchain-r/test
# Deadsnakes - python
RUN add-apt-repository -y ppa:deadsnakes/ppa
# LLVM
RUN curl -fsSL --connect-timeout 5 https://apt.llvm.org/llvm-snapshot.gpg.key | tee /etc/apt/trusted.gpg.d/apt.llvm.org.asc \
    && add-apt-repository -y "deb http://apt.llvm.org/noble/ llvm-toolchain-noble-${LLVM_VERSION} main"

RUN apt-get update && apt-get install -y \
    gcc-${GCC_VERSION} \
    g++-${GCC_VERSION} \
    python${PYTHON_VERSION} \
    python${PYTHON_VERSION}-venv \
    python${PYTHON_VERSION}-dev \
    clang-format-${LLVM_VERSION} \
    clang-tidy-${LLVM_VERSION} \
    clangd-${LLVM_VERSION} \
    && rm -rf /var/lib/apt/lists/*

# update-alternatives to specific versions of gcc/clang
# gcc
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-${GCC_VERSION} 100 \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-${GCC_VERSION} 100

# clang
RUN update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-${LLVM_VERSION} 100 \
    && update-alternatives --install /usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-${LLVM_VERSION} 100 \
    && update-alternatives --install /usr/bin/clangd clangd /usr/bin/clangd-${LLVM_VERSION} 100

WORKDIR /workspace

# Python environment
ENV VIRTUAL_ENV=/opt/venv
RUN python${PYTHON_VERSION} -m venv $VIRTUAL_ENV
ENV PATH="$VIRTUAL_ENV/bin:$PATH"
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

CMD ["/bin/bash"]
