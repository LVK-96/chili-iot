#!/bin/bash

set -e
compiledb make bin
# Remove some ARM/gcc specific options that clang does not understand
sed -i -E 's#-fanalyzer##g' compile_commands.json
sed -i -E 's#-mfix-cortex-m3-ldrd##g' compile_commands.json
