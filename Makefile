.PHONY: run test bench examples gui clean format check

run:
	./build.sh
test: run
	ctest --test-dir build --output-on-failure -j "$$(nproc)"
bench:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DLVTRANS_BUILD_TESTS=OFF -DLVTRANS_BUILD_EXAMPLES=OFF -DLVTRANS_BUILD_GUI=OFF -G "Ninja"
	cmake --build build --config Release -j "$$(nproc)"
	./build/benchmarks/benchmarks
examples:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DLVTRANS_BUILD_EXAMPLES=ON -G "Ninja"
	cmake --build build --config Debug -j "$$(nproc)"
benchA:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DLVTRANS_BUILD_GUI=ON -G "Ninja"
	cmake --build build --target benchA -j "$$(nproc)"
	./build/examples/gui/benchA
benchB:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DLVTRANS_BUILD_GUI=ON -G "Ninja"
	cmake --build build --target benchB -j "$$(nproc)"
	./build/examples/gui/benchB
clean:
	rm -rf build
format:
	find . -type f \( -name "*.cpp" -o -name "*.hpp" \) -exec clang-format -i -style=file {} +
check:
	find . -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" -o -name "*.cc" \) | xargs clang-tidy -fix -- -std=c++20
