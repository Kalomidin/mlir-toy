# Executable name
TARGET = toy

setup:
	@echo "Installing LLVM..."
	@if command -v llvm-config &> /dev/null; then \
		echo "LLVM is already installed."; \
	else \
		echo "Installing LLVM..."; \
		mkdir -p ~/Developer/llvm-project; \
		cd ~/Developer/llvm-project; \
		git clone https://github.com/llvm/llvm-project.git .; \
		mkdir build && cd build; \
		cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=~/Developer/llvm-install ../llvm; \
		make -j$(sysctl -n hw.logicalcpu); \
		make install; \
		echo 'export PATH="$PATH:~/Developer/llvm-install/bin"' >> ~/.zshrc; \
		source ~/.zshrc; \
	fi
	@echo "Setting up the project..."
	@if [ ! -d build ]; then \
		mkdir build; \
	fi
	cd build; \
	cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug ..; \
	make

install:
	make setup
	cp build/$(TARGET) /usr/local/bin/$(TARGET)

# Clean rule to remove compiled files
clean:
	rm -rf build
