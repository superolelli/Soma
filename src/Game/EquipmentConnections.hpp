#pragma once

#include <SFML\Graphics.hpp>

class EquipmentConnections
{
public:

	void Init(int _id);
	void Update();
	void Render(sf::RenderTarget &_target);

	void SetPos(int _x, int _y);

	void ActivateConnection(sf::Color _color);
	void DeactivateConnection(int _deactivatedEmitter);

	const sf::Color& GetColor();

private:

	sf::Sprite connection;

	sf::Shader activationShader;
	sf::Shader deactivationShader;

	sf::RenderTexture resultTexture;

	sf::Glsl::Vec2 emitter[2];
	sf::Color color;

	float elapsedTime;

	const float ACTIVATION_TIME = 3.0;
	const float DEACTIVATION_TIME = 1.0;

	bool performingActivation;
	bool performingDeactivation;

	bool currentlyActive;

	sf::Vector2f position;
};