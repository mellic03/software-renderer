#!/bin/bash
find './src/' \( -name "*.c" -o -name "*.h" \) -not -path './src/include/*' | xargs wc -l