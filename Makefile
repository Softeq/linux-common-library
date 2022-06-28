ifdef OECORE_NATIVE_SYSROOT
  # setup cmake env in the yocto SDK environment
  CMAKE_FLAGS := -DCMAKE_TOOLCHAIN_FILE=${OECORE_NATIVE_SYSROOT}/usr/share/cmake/OEToolchainConfig.cmake
endif

BUILD_DIR = build
THIS_DIR = $(PWD)

# strange verbose: everything or nothing
ifeq ("$(V)","1")
Q :=
vecho = @echo
VERBOSE_CTEST=-V
CMAKE_FLAGS += -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON
CMAKE_FLAGS += -DCMAKE_CTEST_ARGUMENTS=-V
else
Q := @
vecho = @true
VERBOSE_CTEST=
CMAKE_FLAGS += -DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF
endif

DOCKER_BINARY = docker
DOCKER_IMAGE_NAME = softeq-common-library

ifneq (,$(wildcard /.dockerenv))
# Launch inside the docker
define DOCKER_WRAPPER
    ${1}
endef
else
THIS_DIR=/workdir
# Launch outside the docker
define DOCKER_WRAPPER
    CMAKE_VOL=${PWD} CMAKE_IMAGE=${DOCKER_IMAGE_NAME} cmake_docker "${1}"
endef
endif

-include custom.mk

.PHONY: help
help:
	@echo ${BUILD_DIR}
	@echo "The following are valid targets:"
	@echo "  dev           	- build developer version"
	@echo "  prod          	- build production version"
	@echo "  clean         	- clean all version"
	@echo "  test          	- run unit tests"
	@echo "  test_memcheck 	- run unit tests with valgrind"
	@echo "  coverage      	- get test coverage report"
	@echo "  docker        	- build docker image"
	@echo "  shell         	- launch bash"
	@echo "  help          	- get this help"
	@echo "Target options (optional):"
	@echo "  V=1           	- verbose mode"
	@echo "  P=            	- pass additional params to the target command"
	@echo "Example:"
	@echo " make test V=1 (rebuild with verbose and run tests in verbose mode)"

# compile DEVELOP version
.PHONY: dev
dev:
	${Q}$(call DOCKER_WRAPPER, \
		mkdir -p ${BUILD_DIR} && \
		cd ${BUILD_DIR} && \
		cmake ${CMAKE_FLAGS} -DCMAKE_BUILD_TYPE=Debug ${THIS_DIR} && \
		make $(P) \
	)

# compile RELEASE version
.PHONY: prod
prod:
	${Q}$(call DOCKER_WRAPPER, \
		mkdir -p ${BUILD_DIR} && \
		cd ${BUILD_DIR} && \
		cmake ${CMAKE_FLAGS} -DCMAKE_BUILD_TYPE=Release ${THIS_DIR} && \
		make $(P) \
	)

.PHONY: clean
clean:
	${Q}$(call DOCKER_WRAPPER, \
		rm -rf ${BUILD_DIR} \
	)

.PHONY: test
test: dev
	${Q}$(call DOCKER_WRAPPER, \
		cd ${BUILD_DIR} && \
		make test ARGS="${VERBOSE_CTEST}" $(P) \
	)

#actually it required that the build system (CMakeList) implements coverage support (it is project specific)
.PHONY: coverage
coverage:
	${Q}$(call DOCKER_WRAPPER, \
		mkdir -p ${BUILD_DIR} && \
		cd ${BUILD_DIR} && \
		cmake ${CMAKE_FLAGS} -DENABLE_TESTING=ON -DCMAKE_BUILD_TYPE=Debug ${THIS_DIR} && \
		make all test coverage $(P) \
	)

.PHONY: test_memcheck
test_memcheck: dev
	${Q}$(call DOCKER_WRAPPER, \
		mkdir -p ${BUILD_DIR} && \
		cd ${BUILD_DIR} && \
		make test_memcheck $(P) \
	)

# build and install docker image
.PHONY: docker
docker:
	${DOCKER_BINARY} build $(P) -t ${DOCKER_IMAGE_NAME} .

# build and install docker image
.PHONY: shell
shell:
	${Q}$(call DOCKER_WRAPPER, \
		bash \
	)

