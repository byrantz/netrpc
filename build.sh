#!/bin/bash

set -e

function create_dir() {
  if [ ! -d $1 ]; then
    mkdir $1
  fi
}

# 如果没有build目录，创建该目录
create_dir 'build'
create_dir 'lib'
create_dir 'bin'
create_dir 'log'

rm -rf `pwd`/build/*

cd `pwd`/build &&
    cmake .. &&
    make