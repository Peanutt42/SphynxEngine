#include "pch.hpp"

TEST(EngineUtils, BufferViewTest) {
	Sphynx::BufferView bufferView;

	EXPECT_EQ(bufferView.Data, nullptr);
	EXPECT_EQ(bufferView.Size, 0);

	std::vector<int> vector(10);
	bufferView = Sphynx::BufferView(vector);
	EXPECT_EQ(bufferView.As<int>(), vector.data());
	EXPECT_EQ(bufferView.Size, vector.size() * sizeof(int));
	EXPECT_EQ(bufferView.GetSize<int>(), vector.size());

	int array[10];
	bufferView = Sphynx::BufferView(array);
	EXPECT_EQ(bufferView.As<int>(), array);
	EXPECT_EQ(bufferView.Size, std::size(array) * sizeof(int));
	EXPECT_EQ(bufferView.GetSize<int>(), std::size(array));

	std::array<int, 10> std_array;
	bufferView = Sphynx::BufferView(std_array);
	EXPECT_EQ(bufferView.As<int>(), std_array.data());
	EXPECT_EQ(bufferView.Size, std_array.size() * sizeof(int));
	EXPECT_EQ(bufferView.GetSize<int>(), std_array.size());
}