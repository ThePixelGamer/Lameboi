#pragma ocne

#include <array>
#include <bit>
#include <concepts>

template <typename T, size_t N> requires (std::popcount(N) == 1)
struct FIFO {
	std::array<T, N> data{};
	size_t head, tail;
	size_t count;

	FIFO() { reset(); }

	void push(T t) {
		if (size() != data.size()) {
			data[head] = std::move(t);
			head = (head + 1) & N - 1;
			count++;
		}
	}

	T pop() {
		if (size() == 0) return {};
		
		T out = data[tail];
		tail = (tail + 1) & N - 1;
		count--;
		return out;
	}

	void reset() {
		head = 0;
		tail = 0;
		count = 0;
	}

	size_t size() {
		return count;
	}

	T& operator[](size_t i) {
		return data[(tail + i) & N - 1];
	}
};

