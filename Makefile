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
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DLVTRANS_BUILD_GUI=ON -G "Ninja"
	cmake --build build --target gui -j "$$(nproc)"
	./build/examples/gui/gui
bench-gui:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DLVTRANS_BUILD_GUI=ON -G "Ninja"
	cmake --build build --target bench -j "$$(nproc)"
	./build/examples/gui/bench $(PLANT) $(DT)
clean:
	rm -rf build
format:
	fd -t f -e c -e cpp -e h -e hpp . src/ include/ examples/ -x clang-format -i
check:
	fd -i . src/ include/ examples/ | xargs -r -n 1 clang-tidy -fix -p build --extra-arg=-std=c++20
