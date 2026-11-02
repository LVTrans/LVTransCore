.PHONY: run test bench examples gui benchA benchB clean format check

run:
	./build.sh
test:
	cmake -S . -B build -DLVTRANS_BUILD_TESTS=ON -G "Ninja"
	cmake --build build --target tests -j "$$(nproc)"
	ctest --test-dir build --output-on-failure -j "$$(nproc)"
bench:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DLVTRANS_BUILD_TESTS=OFF -DLVTRANS_BUILD_EXAMPLES=OFF -DLVTRANS_BUILD_GUI=OFF -G "Ninja"
	cmake --build build --config Release -j "$$(nproc)"
	./build/benchmarks/benchmarks
examples:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DLVTRANS_BUILD_EXAMPLES=ON -G "Ninja"
	cmake --build build --config Debug -j "$$(nproc)"
gui:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DLVTRANS_BUILD_GUI=ON -G "Ninja"
	cmake --build build --target gui -j "$$(nproc)"
	./build/examples/gui/gui
bench-gui:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DLVTRANS_BUILD_GUI=ON -G "Ninja"
	cmake --build build --target bench -j "$$(nproc)"
	./build/examples/gui/bench
clean:
	rm -rf build
format:
	find . -type f \( -name "*.cpp" -o -name "*.hpp" \) -exec clang-format -i -style=file {} +
check:
	find . -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" -o -name "*.cc" \) | xargs clang-tidy -fix -- -std=c++20
