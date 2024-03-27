#!/usr/bin/bash


while [[ $# > 0 ]]; do
  case $1 in
    -r|--run)
      RUN=1
      shift
      ;;
    *)
      shift
      ;;
  esac
done

mkdir -p build
pushd build
make -j4
popd

if [[ $RUN && $? ]]; then
build/tetris
fi

