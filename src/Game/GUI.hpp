#pragma once

#include "../Framework/Gameengine.hpp"

class GUI
{
public:
	virtual ~GUI() {}

	virtual void Update() = 0;
	virtual void Render() = 0;
};