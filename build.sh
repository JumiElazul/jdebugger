#!/usr/bin/bash

cmake -S . -B bin
cd bin
ninja all
