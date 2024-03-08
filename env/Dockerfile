# Base image
FROM ubuntu:22.04

# Install Ubuntu packages
RUN apt-get update && DEBIAN_FRONTEND=noninteractive \
    apt-get install --yes \
        libncurses5 \
        locales \
        make \
        cmake \
        curl \
        python3 \
        python3-pip \
        python3-venv && \
    rm -rf /var/lib/apt/lists/*

# Install ARM GCC toolchain
ARG COMPILER
RUN test -n "$COMPILER"
RUN curl -o /tmp/gcc-arm-none-eabi-x86_64-linux.tar.bz2 $COMPILER
RUN mkdir -p /usr/bin/gcc-arm-none-eabi && \
tar -xjf /tmp/gcc-arm-none-eabi-x86_64-linux.tar.bz2 --strip-components=1 -C /usr/bin/gcc-arm-none-eabi/ && \
rm -f /tmp/gcc-arm-none-eabi-x86_64-linux.tar.bz2

# Install Segger JLink tools
ARG DEBUGGER
RUN test -n "$DEBUGGER"
RUN curl -X POST -F "accept_license_agreement=accepted" -o /tmp/JLink_Linux_x86_64.tgz $DEBUGGER
RUN mkdir -p /usr/bin/jlink && \
tar -xzf /tmp/JLink_Linux_x86_64.tgz --strip-components=1 -C /usr/bin/jlink/ && \
rm -f /tmp/JLink_Linux_x86_64.tgz

# Add tools to path
ENV PATH="/usr/bin/gcc-arm-none-eabi/bin:/usr/bin/jlink:$PATH"

# Set the locale
RUN locale-gen en_US.UTF-8
ENV LANG=en_US.UTF-8
ENV LANGUAGE=en_US:en
ENV LC_ALL=en_US.UTF-8