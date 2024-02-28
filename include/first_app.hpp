#pragma once

#include "lve_window.hpp"

namespace lve {
	class FirstApp {
		public:
		static constexpr int WIDTH = 1200;
		static constexpr int HEIGHT = 675;

		void run();
		
		private:
			LveWindow lveWindow{WIDTH, HEIGHT, "Hello Vulkan!"};
	};
}