#pragma once

#include "../engine/Gameengine.hpp"

class GUI
{
public:
	virtual ~GUI() {}

	virtual void Update() = 0;
	virtual void Render() = 0;
};