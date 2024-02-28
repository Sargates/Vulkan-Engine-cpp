#include "first_app.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

using std::cout, std::cerr, std::endl;

int main() {
	lve::FirstApp app{};

	try {
		app.run();
	} catch (const std::exception &e) {
		cout << e.what() << endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}