#include "pch.hpp"

int main(int argc, char** argv) {
	Sphynx::Platform::SetWorkingDirToExe();
	// SphynxEngine/bin/${platform}/${config}/Platform/Tests/Tests.exe
	Sphynx::Platform::SetWorkingDirToParentFolder(5);

	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}