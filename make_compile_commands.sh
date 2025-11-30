#!/bin/bash

set -e

# Activate virtual environment if it exists
if [ -d ".venv" ]; then
    source .venv/bin/activate
fi

# Generate compile commands for main binary
compiledb -o compile_commands_target.json -- make bin 

# Remove some ARM/gcc specific options that clang does not understand
sed -i -E 's#-fanalyzer##g' compile_commands_target.json
sed -i -E 's#-mfix-cortex-m3-ldrd##g' compile_commands_target.json

# Generate compile commands for test files (appends to existing file by default)
compiledb -o compile_commands_native.json -- make build/tests/runner 

echo "Successfully generated compile_commands_target.json and compile_commands_native.json with both main and test files"
echo "Linking compile_commands.json to compile_commands_target.json"
ln -sf compile_commands_target.json compile_commands.json
