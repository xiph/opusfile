#!/bin/sh

# script to build a version string

GIT_VERSION=$(git describe --tags --match 'v*' --dirty 2> /dev/null)
GIT_VERSION=$(echo ${GIT_VERSION} | sed 's/^v//')
if test -z ${GIT_VERSION}; then
  VERSION='unknown'
else
  VERSION=${GIT_VERSION}
fi

/bin/echo -n ${VERSION}
