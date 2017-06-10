#!/bin/sh
ARPC=/home/edje/projects/arpc
${ARPC}/scripts/aprotoc.py < flower_service.proto > flower_service.h

x86_64-unknown-cloudabi-c++ -o flower flower.cc -std=c++1z -I${ARPC}/include
