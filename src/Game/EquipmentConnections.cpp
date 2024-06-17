#include "EquipmentConnections.hpp"
#include "Resources\TextureManager.hpp"
#include "../Framework/Time.hpp"
#include <GL\glew.h>


EquipmentConnections::EquipmentConnections(int _id)
	: elapsedTime(0.0)
	, color(sf::Color::Black)
	, performingActivation(false)
	, performingDeactivation(false)
	, currentlyActive(false)
{
	glewInit();  // TODO: Does this have to happen every time? Probably not.

	if (_id == 0)
	{
		connection.setTexture(g_pTextures->inventoryConnection[2]);
		emitter[0] = { 0.005f, 0.98f };
		emitter[1] = { 0.98f, 0.98f };
	}
	else if (_id == 1)
	{
		connection.setTexture(g_pTextures->inventoryConnection[1]);
		emitter[0] = { 0.98f, 0.98f };
		emitter[1] = { 0.98f, 0.005f };
	}
	else if (_id == 2)
	{
		connection.setTexture(g_pTextures->inventoryConnection[0]);
		emitter[1] = { 0.005f, 0.005f };
		emitter[0] = { 0.98f, 0.005f };
	}
	else if (_id == 3)
	{
		connection.setTexture(g_pTextures->inventoryConnection[3]);
		emitter[0] = { 0.005f, 0.005f };
		emitter[1] = { 0.005f, 0.98f };
	}

	resultTexture.create(connection.getGlobalBounds().width, connection.getGlobalBounds().height);
	resultTexture.clear();
	resultTexture.setSmooth(false);

	auto resultTextureHandle = resultTexture.getTexture().getNativeHandle();
	glBindTexture(GL_TEXTURE_2D, resultTextureHandle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, connection.getGlobalBounds().width, connection.getGlobalBounds().height, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	activationShader.loadFromFile("Data/Shader/equipment_connections_activate.frag", sf::Shader::Fragment);
	activationShader.setUniform("u_resolution", sf::Glsl::Vec2(connection.getGlobalBounds().width, connection.getGlobalBounds().height));
	activationShader.setUniform("u_baseTexture", g_pTextures->inventoryConnection[_id]);
	activationShader.setUniformArray("u_emitter", emitter, 2);

	deactivationShader.loadFromFile("Data/Shader/equipment_connections_deactivate.frag", sf::Shader::Fragment);
	deactivationShader.setUniform("u_resolution", sf::Glsl::Vec2(connection.getGlobalBounds().width, connection.getGlobalBounds().height));
	deactivationShader.setUniform("u_baseTexture", g_pTextures->inventoryConnection[_id]);

	resultTexture.draw(connection, &deactivationShader);
	resultTexture.display();
}



void EquipmentConnections::Update()
{
	elapsedTime += g_pTimer->GetElapsedTimeSinceLastUpdateAsSeconds();

	if (performingDeactivation)
	{
		sf::Glsl::Vec4 emitterColor = sf::Glsl::Vec4(color.r / 5.0f, color.g / 5.0f, color.b / 5.0f, 1.0);
		connection.setPosition(0, 0);

		deactivationShader.setUniform("u_progress", std::clamp(elapsedTime / DEACTIVATION_TIME, 0.0f, 1.0f));
		deactivationShader.setUniform("u_emitterColor", emitterColor);

		resultTexture.clear(sf::Color(0,0,0,0));
		resultTexture.draw(connection, &deactivationShader);
		resultTexture.display();

		if (elapsedTime >= DEACTIVATION_TIME)
			performingDeactivation = false;
	}
	else if (performingActivation)
	{
		sf::Glsl::Vec4 emitterColor = sf::Glsl::Vec4(color.r / 5.0f, color.g / 5.0f, color.b / 5.0f, 1.0);
		connection.setPosition(0, 0);

		activationShader.setUniform("u_progress", std::clamp(elapsedTime / ACTIVATION_TIME, 0.0f, 1.0f));
		activationShader.setUniform("u_emitterColor", emitterColor);

		resultTexture.clear(sf::Color(0,0,0,0));
		resultTexture.draw(connection, &activationShader);
		resultTexture.display();

		if (elapsedTime >= ACTIVATION_TIME)
			performingActivation = false;
	}
}


void EquipmentConnections::Render(sf::RenderTarget &_target)
{
	connection.setPosition(position);
	connection.setTexture(resultTexture.getTexture());
	_target.draw(connection);
}

void EquipmentConnections::SetPos(int _x, int _y)
{
	position = sf::Vector2f(_x, _y);
}

const sf::Color& EquipmentConnections::GetColor() 
{ 
	if (!currentlyActive)
		return sf::Color::Black;

	return color; 
}

void EquipmentConnections::ActivateConnection(sf::Color _color)
{
	if (currentlyActive && _color == color)
		return;

	color = _color;
	performingActivation = true;
	performingDeactivation = false;
	currentlyActive = true;
	elapsedTime = 0.0;
}

void EquipmentConnections::DeactivateConnection(int _deactivatedEmitter)
{
	if (!currentlyActive)
		return;

	elapsedTime = 0.0;
	performingDeactivation = true;
	performingActivation = false;
	currentlyActive = false;
	deactivationShader.setUniform("u_emitter", emitter[1 - _deactivatedEmitter]);
}
