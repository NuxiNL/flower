#!/bin/sh
exec find flower \( -name '*.cc' -o -name '*.h' \) -exec clang-format -i {} +
