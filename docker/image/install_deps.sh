#!/bin/sh

set -e -x

apt-get update

DEBIAN_FRONTEND="noninteractive" apt-get -y install tzdata

apt-get install -y \
	ssh \
	make \
	cmake \
	ninja-build \
	git \
	clang-11 \
	clang-format-11 \
	clang-tidy-11 \
	lldb-11 \
	python3 \
	python3-pip \
	python3-venv \
	ca-certificates \
	openssh-server \
	rsync \
	vim \
	gdb \
	wget \
	autoconf \
	iputils-ping \
	binutils-dev

pip3 install \
	click \
	gitpython \
	python-gitlab \
	termcolor \
	virtualenv \
	argcomplete
