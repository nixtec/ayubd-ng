#!/bin/bash

runas="`whoami`"
cmd="./captcha_static_svc"
dir="`dirname $0`"
exec &> "${HOME}/tmp/`basename $cmd`.log"

pushd "${dir}" || exit
date
echo "Going to execute $cmd as user $runas"
make clean all
$cmd &
popd
