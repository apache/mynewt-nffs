#!/bin/bash -x

# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

pushd $HOME
git clone --depth=1 https://github.com/apache/mynewt-newt
[[ $? -ne 0 ]] && exit 1

pushd mynewt-newt && ./build.sh
[[ $? -ne 0 ]] && exit 1

cp newt/newt $HOME/bin
popd
popd

ln -s /usr/bin/gcc-7 $HOME/bin/gcc

mkdir -p repos/apache-mynewt-core
git clone --depth=1 https://github.com/apache/mynewt-core repos/apache-mynewt-core
