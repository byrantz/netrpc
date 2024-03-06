#!/bin/bash

set -e

rm -rf `pwd`/build/*
rm -rf `pwd`/bin/*
rm -rf `pwd`/lib/*
cd `pwd`/build &&
	cmake .. &&
	make