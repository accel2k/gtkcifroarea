
ifeq ($(basename $(notdir $(MAKE))), mingw32-make)
  CMAKE_GENERATOR = "MinGW Makefiles"
  MAKE_DIR = md
  REMOVE_DIR = rd /s /q
else
  CMAKE_GENERATOR = "Unix Makefiles"
  MAKE_DIR = mkdir -p
  REMOVE_DIR = rm -rf
endif

PREFIX ?= /usr

all: release

release:
	@-${MAKE_DIR} build
	@cd build && cmake -G $(CMAKE_GENERATOR) \
                       -D CMAKE_BUILD_TYPE=Release \
                       -D CMAKE_INSTALL_PREFIX=$(PREFIX) \
                       -D CIFRO_AREA_WITH_GTK2=$(WITH_GTK2) \
                       -D CIFRO_AREA_WITH_DOC=$(WITH_DOC) \
                       ..
	@$(MAKE) -C build

debug:
	@-${MAKE_DIR} build
	@cd build && cmake -G $(CMAKE_GENERATOR) \
                       -D CMAKE_BUILD_TYPE=Debug \
                       -D CMAKE_INSTALL_PREFIX=$(PREFIX) \
                       -D CIFRO_AREA_WITH_GTK2=$(WITH_GTK2) \
                       -D CIFRO_AREA_WITH_DOC=$(WITH_DOC) \
                       ..
	@$(MAKE) -C build

install: install-runtime

install-all: release
	@echo "Performing installation"
	@cd build && cmake -D CMAKE_INSTALL_DO_STRIP=YES \
                       -P cmake_install.cmake

install-runtime: release
	@echo "Performing runtime installation"
	@cd build && cmake -D COMPONENT=runtime \
                       -D CMAKE_INSTALL_DO_STRIP=YES \
                       -P cmake_install.cmake

install-development: release
	@echo "Performing development installation"
	@cd build && cmake -D COMPONENT=development \
                       -P cmake_install.cmake

install-test: install-runtime
	@echo "Performing test installation"
	@cd build && cmake -D COMPONENT=test \
                       -D CMAKE_INSTALL_DO_STRIP=YES \
                       -P cmake_install.cmake

clean:
	@echo "Cleaning build directory"
	-@$(MAKE) -C build clean

distclean: clean
	@echo "Removing build directory"
	-@${REMOVE_DIR} bin
	-@${REMOVE_DIR} build
