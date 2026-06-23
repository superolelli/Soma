#include "Scrollbar.hpp"


VerticalScrollbar::VerticalScrollbar(int _width, int _maximumHeight)
	: numberOfSteps(1)
	, currentStep(0)
	, maximumHeight(_maximumHeight)
	, isScrolling(false)
{
	scrollbarShape.setSize(sf::Vector2f(_width, _maximumHeight - 10));
	scrollbarShape.setOutlineColor(sf::Color::Black);
	scrollbarShape.setOutlineThickness(5);
}

void VerticalScrollbar::Update(CGameEngine & _engine)
{
	if (_engine.GetButtonstates(ButtonID::Left) == Pressed && scrollbarShape.getGlobalBounds().contains(sf::Vector2f(_engine.GetMousePos())))
		isScrolling = true;
	else if (_engine.GetButtonstates(ButtonID::Left) == Released)
	{
		SetScrollbarToCurrentStep();
		isScrolling = false;
	}

	int stepHeight = maximumHeight / numberOfSteps;;

	if (_engine.GetMouseWheelMovement())
	{
		scrollbarShape.move(sf::Vector2f(0.f, static_cast<float>(-_engine.GetMouseWheelMovement() * stepHeight)));
		EnsureBounds();
	}
	else if (isScrolling)
	{
		scrollbarShape.move(sf::Vector2f(0, _engine.GetMouseMovement().y));
		EnsureBounds();
	}

	currentStep = (scrollbarShape.getGlobalBounds().position.y + scrollbarShape.getGlobalBounds().size.y / 2 - upperYBoundary) / stepHeight;
}


void VerticalScrollbar::EnsureBounds()
{
	if (scrollbarShape.getGlobalBounds().position.y + scrollbarShape.getGlobalBounds().size.y + scrollbarShape.getOutlineThickness() * 2 > upperYBoundary + maximumHeight)
		scrollbarShape.setPosition(sf::Vector2f(leftXBoundary, upperYBoundary + maximumHeight - scrollbarShape.getGlobalBounds().size.y));

	if (scrollbarShape.getGlobalBounds().position.y < upperYBoundary)
		scrollbarShape.setPosition(sf::Vector2f(leftXBoundary, upperYBoundary));
}


void VerticalScrollbar::SetScrollbarToCurrentStep()
{
	scrollbarShape.setPosition(sf::Vector2f(leftXBoundary, upperYBoundary + currentStep * scrollbarShape.getGlobalBounds().size.y));
}

void VerticalScrollbar::Render(sf::RenderTarget & _target)
{
	_target.draw(scrollbarShape);
}

void VerticalScrollbar::SetColor(sf::Color _color)
{
	scrollbarShape.setFillColor(_color);
}

void VerticalScrollbar::SetOutlineThickness(float _thickness)
{
	scrollbarShape.setOutlineThickness(_thickness);
}

void VerticalScrollbar::SetNumberOfSteps(int _steps)
{
	numberOfSteps = _steps;
	scrollbarShape.setSize(sf::Vector2f(scrollbarShape.getSize().x, maximumHeight / _steps - scrollbarShape.getOutlineThickness() * 2));
	scrollbarShape.setPosition(sf::Vector2f(leftXBoundary, upperYBoundary));
}

int VerticalScrollbar::GetNumberOfSteps()
{
	return numberOfSteps;
}

int VerticalScrollbar::GetCurrentStep()
{
	return currentStep;
}


void VerticalScrollbar::SetPos(float _x, float _y)
{
	scrollbarShape.setPosition(sf::Vector2f(_x, _y));
	leftXBoundary = _x;
	upperYBoundary = _y;
}
