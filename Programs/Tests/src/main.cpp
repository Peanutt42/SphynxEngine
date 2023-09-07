#include "pch.hpp"

int main(int argc, char** argv) {
	Sphynx::Platform::SetWorkingDirToExe();

	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}