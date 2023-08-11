#include "pch.hpp"

int main(int argc, char** argv) {
	if (argc >= 1) {
		std::filesystem::path exePath = std::filesystem::path(argv[0]);
		if (std::filesystem::exists(exePath))
			std::filesystem::current_path(exePath.parent_path().parent_path().parent_path().parent_path().parent_path().parent_path());
	}

	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}