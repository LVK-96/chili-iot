#!/bin/bash

set -e

# Activate virtual environment if it exists
if [ -d ".venv" ]; then
    source .venv/bin/activate
fi

# Generate compile commands for main binary
compiledb make bin

# Remove some ARM/gcc specific options that clang does not understand
sed -i -E 's#-fanalyzer##g' compile_commands.json
sed -i -E 's#-mfix-cortex-m3-ldrd##g' compile_commands.json

# Generate compile commands for test files (appends to existing file by default)
compiledb make build/tests/runner

echo "Successfully generated compile_commands.json with both main and test files"
