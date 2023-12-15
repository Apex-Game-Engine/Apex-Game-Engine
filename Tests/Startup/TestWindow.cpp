#include <apex_pch.h>
#include <Apex/Game.h>

class MyGame : public apex::Game
{
public:
	void run() override {}
	void stop() override {}
};

apex::UniquePtr<apex::Game> apex::Game::Construct()
{
	return apex::static_unique_cast<Game>(apex::make_unique<MyGame>());
}

#include <Startup.h>
