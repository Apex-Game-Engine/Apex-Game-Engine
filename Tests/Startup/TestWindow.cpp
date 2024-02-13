#include <apex_pch.h>
#include <Apex/Game.h>
#include <Apex/Application.h>

#include "TestGame.h"


apex::UniquePtr<apex::Game> apex::Game::Construct()
{
	return apex::static_unique_cast<Game>(apex::make_unique<MyGame>());
}

#include <Startup.h>
