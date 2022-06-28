#!/bin/bash

set -e

WORKDIR=sonar

rm -rf ${WORKDIR}
mkdir ${WORKDIR}
pushd ${WORKDIR} > /dev/null
cmake ${CMAKE_FLAGS} -DENABLE_TESTING=ON -DENABLE_EXAMPLES=ON -DCMAKE_BUILD_TYPE=Debug ..
/opt/build-wrapper-linux-x86/build-wrapper-linux-x86-64 --out-dir bw-outputs make all
make test coverage
popd > /dev/null

export SONAR_USER_HOME=.sonar
/opt/sonar-scanner-4.5.0.2216-linux/bin/sonar-scanner \
  -Dsonar.projectKey=org.sonarqube:cpp-build-wrapper \
  -Dsonar.host.url=https://sonar.softeq.com \
  -Dsonar.login=b3bfe201631551dae94ba61dd72a5bdc9e78ca6e \
  -Dsonar.coverageReportPaths=${WORKDIR}/Coverage/sonarcube.xml \
  -Dsonar.cfamily.build-wrapper-output=${WORKDIR}/bw-outputs

