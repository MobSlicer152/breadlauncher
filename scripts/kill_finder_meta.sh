#!/usr/bin/env bash

rm -f `find . -type f | grep -i '\._'`
rm -f `find . -type f | grep -i '\.DS_Store'`
