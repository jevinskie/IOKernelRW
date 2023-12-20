KEXT_TARGET := IOKernelRW
TEST_TARGET := test-rw-strchr
SRC    := src

# Don't use ?= with $(shell ...)
ifndef CXX_FLAGS
CXX_FLAGS := --std=gnu++17 -Wall -O3 -nostdinc -nostdlib -mkernel -DKERNEL -isystem $(shell xcrun --show-sdk-path)/System/Library/Frameworks/Kernel.framework/Headers -Wl,-kext -lcc_kext $(CXXFLAGS)
endif

.PHONY: all install clean

all: $(KEXT_TARGET).kext/Contents/_CodeSignature/CodeResources $(TEST_TARGET)

$(KEXT_TARGET).kext/Contents/MacOS/$(KEXT_TARGET): $(SRC)/*.cpp $(SRC)/*.h | $(KEXT_TARGET).kext/Contents/MacOS
	$(CXX) -arch arm64e -arch x86_64 -o $@ $(SRC)/*.cpp $(CXX_FLAGS)

$(KEXT_TARGET).kext/Contents/Info.plist: misc/Info.plist | $(KEXT_TARGET).kext/Contents
	cp -f $^ $@

$(KEXT_TARGET).kext/Contents/_CodeSignature/CodeResources: $(KEXT_TARGET).kext/Contents/MacOS/$(KEXT_TARGET) $(KEXT_TARGET).kext/Contents/Info.plist
	codesign -s - -f $(KEXT_TARGET).kext

$(KEXT_TARGET).kext/Contents $(KEXT_TARGET).kext/Contents/MacOS:
	mkdir -p $@

$(TEST_TARGET): test-rw-strchr.cpp test-rw-strchr-ent.xml
	$(CXX) -o $@ test-rw-strchr.cpp -std=c++20 -arch arm64 -arch arm64e -arch x86_64 -framework IOKit -Wall -Wextra
	codesign -s - --entitlements test-rw-strchr-ent.xml $@

install: all
	sudo cp -R $(KEXT_TARGET).kext /Library/Extensions/

clean:
	rm -rf $(KEXT_TARGET).kext
	rm -f $(TEST_TARGET)
