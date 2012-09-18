#!/bin/sh

# script to build a version string

GIT_VERSION=$(git describe --tags --dirty 2> /dev/null)
if test -z ${GIT_VERSION}; then
  VERSION='unknown'
else
  VERSION=${GIT_VERSION}
fi

echo ${VERSION}
