CC=gcc
# Adjust CFLAGS and LDFLAGS if your libraries are in non-standard locations
# e.g., CFLAGS+=-I/opt/local/include LDFLAGS+=-L/opt/local/lib
CFLAGS=-fPIC -Wall -Werror -Imain/include
LDFLAGS=-shared

# List of libraries to link against
LDLIBS=-lyaml -largon2 -lcyaml

# --- Main Build ---
TARGET=target/pam_wewe.so
OBJS=target/pam_wewe.o target/wewe_config.o target/wewe_net.o

# --- Test Build ---
TEST_TARGET = target/test_runner
TEST_LIB_SRCS = main/src/wewe_config.c main/src/pam_wewe.c
TEST_MAIN_SRCS = $(wildcard test/src/*.c)

TEST_LIB_OBJS = $(patsubst main/src/%.c,target/test_%.o,$(TEST_LIB_SRCS))
TEST_MAIN_OBJS = $(patsubst test/src/%.c,target/test_%.o,$(TEST_MAIN_SRCS))
TEST_OBJS = $(TEST_LIB_OBJS) $(TEST_MAIN_OBJS)

TEST_CFLAGS = -Imain/include -Wall -Werror `pkg-config --cflags check`
TEST_LDFLAGS = `pkg-config --libs check` -lyaml -largon2 -lcyaml

.PHONY: all build-test test clean deb docker-build sonar-scan download-build-wrapper cppcheck docker-deb docker-cppcheck

all: build_dir $(TARGET)

build_dir:
	@mkdir -p target

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

target/%.o: main/src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# --- Test Targets ---

build-test: $(TEST_TARGET)

test: build-test
	@echo "--- Running Tests ---"
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJS)
	@mkdir -p $(dir $@)
	$(CC) -o $@ $^ $(TEST_LDFLAGS)

# Rule for test-specific object files (from test/src)
target/test_%.o: test/src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) -c $< -o $@

# Rule for library object files compiled for tests (from src)
target/test_%.o: main/src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) -c $< -o $@

CPPCHECK := cppcheck
CPPCHECK_FLAGS := --enable=all --std=c11 -Imain/include --suppress=missingIncludeSystem --suppress=unusedFunction --error-exitcode=1

.PHONY: cppcheck
cppcheck:
	@echo "Running Cppcheck static analysis..."
	$(CPPCHECK) $(CPPCHECK_FLAGS) main/src/
	@echo "Cppcheck analysis complete."

# --- Cleanup ---
clean:
	rm -rf target/*

# --- Packaging ---
PACKAGE_NAME = pam-wewe
PACKAGE_VERSION = 0.1
DEB_ARCH ?= $(shell dpkg --print-architecture)
LIB_DIR ?= $(shell gcc -print-multiarch 2>/dev/null || dpkg-architecture -qDEB_HOST_MULTIARCH)
deb: all
	@echo "Building Debian package for arch $(DEB_ARCH)..."
	$(eval DEB_STAGING_DIR := target/deb/$(PACKAGE_NAME)-$(PACKAGE_VERSION)-$(DEB_ARCH))
	rm -rf $(DEB_STAGING_DIR)
	mkdir -p $(DEB_STAGING_DIR)/DEBIAN
	mkdir -p $(DEB_STAGING_DIR)/lib/$(LIB_DIR)/security

	# Copy compiled library
	cp $(TARGET) $(DEB_STAGING_DIR)/lib/$(LIB_DIR)/security/pam_wewe.so

	# Copy control files
	# Dynamically set the architecture in the control file
	sed -e "s/^Architecture:.*/Architecture: $(DEB_ARCH)/" -e "s/^Version:.*/Version: $(PACKAGE_VERSION)/" debian/control > $(DEB_STAGING_DIR)/DEBIAN/control
	cp debian/postinst $(DEB_STAGING_DIR)/DEBIAN/
	cp debian/postrm $(DEB_STAGING_DIR)/DEBIAN/

	# Copy copyright file
	mkdir -p $(DEB_STAGING_DIR)/usr/share/doc/$(PACKAGE_NAME)
	cp debian/copyright $(DEB_STAGING_DIR)/usr/share/doc/$(PACKAGE_NAME)/copyright

	# Set permissions for scripts
	chmod 0755 $(DEB_STAGING_DIR)/DEBIAN/postinst
	chmod 0755 $(DEB_STAGING_DIR)/DEBIAN/postrm
	
	# Build the deb package. dpkg-deb creates a file with underscores.
	dpkg-deb --build $(DEB_STAGING_DIR) target/

	# Rename the package to a consistent format with dashes
	@mv -f target/$(PACKAGE_NAME)_$(PACKAGE_VERSION)_$(DEB_ARCH).deb target/$(PACKAGE_NAME)-$(PACKAGE_VERSION)-$(DEB_ARCH).deb

	@echo "Successfully generated: target/$(PACKAGE_NAME)-$(PACKAGE_VERSION)-$(DEB_ARCH).deb"

# --- Docker Build Target ---
DOCKER_IMAGE_NAME ?= pam-wewe-builder

.PHONY: docker-build
docker-build: build_dir
	@echo "--- Building Docker image: $(DOCKER_IMAGE_NAME) ---"
	@docker build -t $(DOCKER_IMAGE_NAME) .
	@docker run --rm -v "$(CURDIR)/target:/app/target" $(DOCKER_IMAGE_NAME) make all
	@echo "\n✅ Build complete. Artifacts are in the ./target/ directory."

.PHONY: docker-deb
docker-deb: build_dir
	@echo "--- Building Docker image: $(DOCKER_IMAGE_NAME) ---"
	@docker build -t $(DOCKER_IMAGE_NAME) .
	@docker run --rm -v "$(CURDIR)/target:/app/target" $(DOCKER_IMAGE_NAME) make deb
	@echo "\n✅ Build complete. Artifacts are in the ./target/ directory."

.PHONY: docker-test
docker-test: build_dir
	@echo "--- Building Docker image: $(DOCKER_IMAGE_NAME) ---"
	@docker build -t $(DOCKER_IMAGE_NAME) .
	@docker run --rm -v "$(CURDIR)/target:/app/target" $(DOCKER_IMAGE_NAME) make test
	@echo "\n✅ Build complete. Artifacts are in the ./target/ directory."

.PHONY: docker-cppcheck
docker-cppcheck: build_dir
	@echo "--- Building Docker image: $(DOCKER_IMAGE_NAME) ---"
	@docker build -t $(DOCKER_IMAGE_NAME) .
	@docker run --rm -v "$(CURDIR)/target:/app/target" $(DOCKER_IMAGE_NAME) make cppcheck
	@echo "\n✅ Build complete. Artifacts are in the ./target/ directory."

# --- SonarQube Analysis ---
SONAR_URL ?= http://localhost:9000
BUILD_WRAPPER_ZIP_NAME=build-wrapper-linux-x86.zip
BUILD_WRAPPER_DIR=./.bw
BUILD_WRAPPER_EXECUTABLE=$(BUILD_WRAPPER_DIR)/build-wrapper-linux-x86/build-wrapper-linux-x86-64

.PHONY: download-build-wrapper
download-build-wrapper:
	@echo "--- Downloading SonarQube Build Wrapper ---"
	@mkdir -p $(BUILD_WRAPPER_DIR)
	@wget -q -O $(BUILD_WRAPPER_DIR)/$(BUILD_WRAPPER_ZIP_NAME) $(SONAR_URL)/static/cpp/$(BUILD_WRAPPER_ZIP_NAME)
	@unzip -qo $(BUILD_WRAPPER_DIR)/$(BUILD_WRAPPER_ZIP_NAME) -d $(BUILD_WRAPPER_DIR)
	@rm $(BUILD_WRAPPER_DIR)/$(BUILD_WRAPPER_ZIP_NAME)
	@echo "Build Wrapper downloaded to $(BUILD_WRAPPER_DIR)"

.PHONY: sonar-scan
sonar-scan: clean download-build-wrapper
	docker build -f docker/Dockerfile.sonar -t pam-wewe-sonar .
	@echo "--- Running build-wrapper inside Docker container (docker run) ---"
	@docker run --rm -v "$(CURDIR):/usr/src" pam-wewe-sonar \
	    $(BUILD_WRAPPER_EXECUTABLE) --out-dir target make
	@echo "--- Running Sonar Scanner ---"
	@if [ -z "$(token)" ]; then \
	    echo "Error: SonarQube token not provided."; \
	    echo "Usage: make sonar-scan token=<YOUR_SONAR_TOKEN>"; \
	    exit 1; \
	fi
	docker run --rm --network=host -e SONAR_TOKEN=$(token) -v "$(CURDIR):/usr/src" pam-wewe-sonar sonar-scanner

