#include "EquipmentConnections.hpp"
#include "Resources\TextureManager.hpp"
#include "../Framework/Time.hpp"
#include <GL\glew.h>


void EquipmentConnections::Init(int _id)
{
	glewInit();

	if (_id == 0)
	{
		connection.setTexture(g_pTextures->inventoryConnection[2]);
		emitter[0] = { 0.005f, 0.98f };
		emitter[1] = { 0.98f, 0.98f };
		emitterDirection[0] = {0.05f, -0.004f};
		emitterDirection[1] = { -0.05f, -0.004f };
	}
	else if (_id == 1)
	{
		connection.setTexture(g_pTextures->inventoryConnection[1]);
		emitter[0] = { 0.98f, 0.98f };
		emitter[1] = { 0.98f, 0.005f };
		emitterDirection[0] = { -0.004f, -0.05f };
		emitterDirection[1] = { -0.004f, 0.05f };
	}
	else if (_id == 2)
	{
		connection.setTexture(g_pTextures->inventoryConnection[0]);
		emitter[1] = { 0.005f, 0.005f };
		emitter[0] = { 0.98f, 0.005f };
		emitterDirection[1] = { 0.05f, 0.004f };
		emitterDirection[0] = { -0.05f, 0.004f };
	}
	else if (_id == 3)
	{
		connection.setTexture(g_pTextures->inventoryConnection[3]);
		emitter[0] = { 0.005f, 0.005f };
		emitter[1] = { 0.005f, 0.98f };
		emitterDirection[0] = { 0.004f, 0.05f };
		emitterDirection[1] = { 0.004f, -0.05f };
	}

	//color = sf::Glsl::Vec4(0.0, 0.0, 0.0, 1.0);


	velocityPassTexture.create(connection.getGlobalBounds().width, connection.getGlobalBounds().height);
	velocityPassTexture.clear();
	velocityPassTexture.setSmooth(true);
	//velocityPassTexture.setRepeated(true);

	auto glTex = velocityPassTexture.getTexture().getNativeHandle();
	glBindTexture(GL_TEXTURE_2D, glTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, connection.getGlobalBounds().width, connection.getGlobalBounds().height, 0, GL_RGBA, GL_FLOAT, NULL);

	colorPassTexture.create(connection.getGlobalBounds().width, connection.getGlobalBounds().height);
	colorPassTexture.clear();
	colorPassTexture.setSmooth(true);
	//texture2.setRepeated(true);

	auto glTex2 = colorPassTexture.getTexture().getNativeHandle();
	glBindTexture(GL_TEXTURE_2D, glTex2);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, connection.getGlobalBounds().width, connection.getGlobalBounds().height, 0, GL_RGBA, GL_FLOAT, NULL);


	velocityPassShader.loadFromFile("Data/Shader/equipment_connections_velocity.frag", sf::Shader::Fragment);
	velocityPassShader.setUniform("u_resolution", sf::Glsl::Vec2(connection.getGlobalBounds().width, connection.getGlobalBounds().height));
	velocityPassShader.setUniform("u_baseTexture", g_pTextures->inventoryConnection[_id]);
	velocityPassShader.setUniformArray("u_emitter", emitter, 2);
	velocityPassShader.setUniformArray("u_emitterDirection", emitterDirection, 2);

	colorPassShader.loadFromFile("Data/Shader/equipment_connections_color.frag", sf::Shader::Fragment);
	colorPassShader.setUniform("u_resolution", sf::Glsl::Vec2(connection.getGlobalBounds().width, connection.getGlobalBounds().height));
	colorPassShader.setUniform("u_baseTexture", g_pTextures->inventoryConnection[_id]);
	colorPassShader.setUniformArray("u_emitter", emitter, 2);
	//colorPassShader.setUniform("u_emitterColor", emitterColor);

	deactivationShader.loadFromFile("Data/Shader/equipment_connections_deactivate.frag", sf::Shader::Fragment);
	deactivationShader.setUniform("u_resolution", sf::Glsl::Vec2(connection.getGlobalBounds().width, connection.getGlobalBounds().height));
	deactivationShader.setUniform("u_baseTexture", g_pTextures->inventoryConnection[_id]);

	//velocityPassTexture.clear();
	//colorPassTexture.clear();

	elapsedTime = 0.0;
	frame = 0;

	deactivationTime = 0.0;
	color = sf::Color::Black;
}



void EquipmentConnections::Update()
{
	if (deactivationTime >= 0.0)
	{
		connection.setPosition(0, 0);
		deactivationShader.setUniform("u_time", 3.0f - static_cast<float>(deactivationTime));
		deactivationShader.setUniform("u_lastTexture", colorPassTexture.getTexture());
		deactivationTime -= g_pTimer->GetElapsedTimeSinceLastUpdateAsSeconds();

		colorPassTexture.draw(connection, &deactivationShader);
		colorPassTexture.display();
		return;
	}


	if (elapsedTime >= 5.0)
		return;

	sf::Glsl::Vec4 emitterColor = sf::Glsl::Vec4(color.r / 5.0f, color.g / 5.0f, color.b / 5.0f, 1.0);

	connection.setPosition(0, 0);

	elapsedTime += g_pTimer->GetElapsedTimeSinceLastUpdateAsSeconds();

	velocityPassShader.setUniform("u_time", static_cast<float>(elapsedTime));
	velocityPassShader.setUniform("u_frame", frame);
	velocityPassShader.setUniform("u_lastTexture", velocityPassTexture.getTexture());

	velocityPassTexture.draw(connection, &velocityPassShader);
	velocityPassTexture.display();

	velocityPassShader.setUniform("u_lastTexture", velocityPassTexture.getTexture());
	velocityPassTexture.draw(connection, &velocityPassShader);
	velocityPassTexture.display();

	velocityPassShader.setUniform("u_lastTexture", velocityPassTexture.getTexture());
	velocityPassTexture.draw(connection, &velocityPassShader);
	velocityPassTexture.display();


	colorPassShader.setUniform("u_time", static_cast<float>(elapsedTime));
	colorPassShader.setUniform("u_textureVelo", velocityPassTexture.getTexture());
	colorPassShader.setUniform("u_textureColor", colorPassTexture.getTexture());
	colorPassShader.setUniform("u_frame", frame);
	colorPassShader.setUniform("u_emitterColor", emitterColor);

	colorPassTexture.draw(connection, &colorPassShader);
	colorPassTexture.display();

	frame++;
}



void EquipmentConnections::Render(sf::RenderTarget &_target)
{
	connection.setPosition(position);
	connection.setTexture(colorPassTexture.getTexture());
	_target.draw(connection);
}



void EquipmentConnections::SetPos(int _x, int _y)
{
	position = sf::Vector2f(_x, _y);
}



void EquipmentConnections::ActivateConnection(sf::Color _color)
{
	if (_color == color)
		return;

	color = _color;

	Restart();
}

void EquipmentConnections::DeactivateConnection(int _deactivatedEmitter)
{
	if (color == sf::Color::Black)
		return;

	color = sf::Color::Black;
	deactivationTime = 3.0;
	deactivationShader.setUniform("u_emitter", emitter[!_deactivatedEmitter]);
	//Restart();
}


void EquipmentConnections::Restart()
{
	frame = 0;
	elapsedTime = 0.0;
}