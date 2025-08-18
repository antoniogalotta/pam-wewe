FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    gcc \
    make \
    libpam-dev \
    libyaml-dev \
    libargon2-dev \
    libcyaml-dev \
    debhelper \
    dpkg-dev \
    check \
    cppcheck \
    pkg-config \
    wget \
    unzip \
    git \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/src
COPY . .

CMD ["/bin/bash"]