#include "pch.hpp"

TEST(EngineUtils, ResultTests) {
	Sphynx::Result<bool, std::string_view> string_view_result;
	string_view_result.set_error("This is a std::string_view");
	EXPECT_TRUE(string_view_result.is_error());

	Sphynx::Result<bool, std::string> string_result = string_view_result;
	EXPECT_TRUE(string_result.is_error());

	EXPECT_EQ(string_result.get_error(), string_view_result.get_error());

	Sphynx::Result<int> int_result(1);
	EXPECT_TRUE(int_result.is_ok());
	EXPECT_EQ(int_result.unwrap(), 1);
	int_result = 5;
	EXPECT_EQ(int_result.unwrap(), 5);
}