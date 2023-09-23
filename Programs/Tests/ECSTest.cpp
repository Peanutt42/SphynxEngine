#include "pch.hpp"
#include "ECS/Registry.hpp"

int counter = 0;

struct Lifetime {
	Lifetime() {
		counter++;
	}
	Lifetime(const Lifetime&) {
		counter++;
	}
	Lifetime(Lifetime&&) {
		counter++;
	}
	~Lifetime() {
		counter--;
	}

	int __data;
};

TEST(Engine, ECS) {
	counter = 0;

	{
		Sphynx::ECS::Registry registry;

		registry.AddComponent<Lifetime>(registry.Create());

		Sphynx::ECS::EntityId e1 = registry.Create();
		registry.AddComponent<Lifetime>(e1);
		EXPECT_TRUE(registry.HasComponent<Lifetime>(e1));

		Sphynx::ECS::EntityId e2 = registry.Create();
		registry.AddComponent(e2, *registry.GetComponent<Lifetime>(e1));
		EXPECT_TRUE(registry.HasComponent<Lifetime>(e2));

		registry.RemoveComponent<Lifetime>(e1);
		EXPECT_FALSE(registry.HasComponent<Lifetime>(e1));

		Sphynx::ECS::EntityId e3 = registry.Dublicate(e2);
		EXPECT_TRUE(registry.HasComponent<Lifetime>(e3));
		registry.RemoveComponent<Lifetime>(e3);
		EXPECT_FALSE(registry.HasComponent<Lifetime>(e3));

		registry.AddComponent(e3, Lifetime{});

		registry.Clear();

		registry.AddComponent(registry.Create(), Lifetime{});
	}

	EXPECT_EQ(counter, 0) << "Lifetime problem, non zero amount of creation/deletion! -> possible memory leak!";
}