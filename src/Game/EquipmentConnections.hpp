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

	sf::Color &GetColor() { return color; }

	void Restart();

private:

	sf::Sprite connection;

	sf::Shader velocityPassShader;
	sf::Shader colorPassShader;
	sf::Shader deactivationShader;

	sf::RenderTexture velocityPassTexture;
	sf::RenderTexture colorPassTexture;

	sf::Glsl::Vec2 emitter[2];
	sf::Glsl::Vec2 emitterDirection[2];
	sf::Color color;

	float elapsedTime;
	int frame;

	float deactivationTime;

	sf::Vector2f position;
};