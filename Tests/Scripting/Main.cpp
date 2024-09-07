#include <apex_pch.h>
#include <Apex/Game.h>
#include <Apex/Application.h>

#include "LuaSimpleExample.h"
#include "LuaStateMachineExample.h"

apex::UniquePtr<apex::Game> apex::Game::Construct()
{
	//return apex::static_unique_cast<Game>(apex::make_unique<LuaStateMachineExample>());
	return apex::static_unique_cast<Game>(apex::make_unique<LuaSimpleExample>());
}

#include <Startup.h>
