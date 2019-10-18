#pragma once


#include <SFML/Graphics.hpp>
#include "../Gameengine.hpp"

class VerticalScrollbar
{
public:

	void Init(int _width, int _maximumHeight);
	void Update(CGameEngine &_engine);
	void Render(sf::RenderTarget &_target);

	void SetColor(sf::Color _color);
	void SetNumberOfSteps(int _steps);
	int GetNumberOfSteps();

	int GetCurrentStep();

	void SetPos(float _x, float _y);

	void SetOutlineThickness(float _thickness);

private:

	bool isScrolling;

	int maximumHeight;

	sf::RectangleShape scrollbarShape;
	int numberOfSteps;
	int currentStep;

	float leftXBoundary;
	float upperYBoundary;

	void EnsureBounds();
	void SetScrollbarToCurrentStep();
};