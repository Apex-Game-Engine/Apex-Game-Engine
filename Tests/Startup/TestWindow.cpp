#include <apex_pch.h>
#include <Apex/Game.h>
#include <Apex/Application.h>

#include "TestGame.h"
#include "ClothSim.h"
#include "InverseKinematics.h"

apex::UniquePtr<apex::Game> apex::Game::Construct()
{
	return apex::static_unique_cast<Game>(apex::make_unique<MyGame>());
	//return apex::static_unique_cast<Game>(apex::make_unique<ClothSim>());
	//return apex::static_unique_cast<Game>(apex::make_unique<IKTrial>());
}

#include <Startup.h>
