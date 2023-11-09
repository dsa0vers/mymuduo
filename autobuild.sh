#!/bin/bash

set -e

# 如果没有mymuduo目录，则创建目录
if [ ! -d `pwd`/build ];then
    mkdir `pwd`/build
fi

rm -rf `pwd`/build/*

cd `pwd`/build &&
    cmake .. &&
    make

cd ..

# 将头文件拷贝到/usr/include/mymuduo ,将so库拷贝到/usr/lib中
if [ ! -d /usr/include/mymuduo ];then
    mkdir /usr/include/mymuduo
fi

for header in `ls *.h`
do
    cp $header /usr/include/mymuduo
done

cp `pwd`/lib/libmymuduo.so /usr/lib

# 刷新，使其生效
ldconfig