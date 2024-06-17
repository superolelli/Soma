#pragma once


#include "../Gameengine.hpp"
#include "../Graphics/Sprite.hpp"

#include <functional>

enum class Buttontypes{Up, Down, Motion_Up, Motion_Down};

class CButton
{
public:

	CButton(sf::Texture const& _texture, Buttontypes _type, std::string const& _str = "");
    ~CButton();

	//Sets the button's position
	void SetPos(int _x, int _y);

	//sets the scale of the button
	void SetScale(float _x, float _y);

	void SetButtontextFont(sf::Font const &_font);
	void SetButtontextCharactersize(int _size);
	void SetButtontextColor(sf::Color const &_color);
	void SetButtontextStyle(sf::Text::Style _style);

	void SetDisabled();
	void SetEnabled();

	void SetCallback(std::function<void()> _callback);

	//sets the buttonstring
	void SetButtonstring(std::string const &_buttonstring);

	bool Update(CGameEngine &_engine);

	//renders the button, returns true if button was pressed
	void Render(sf::RenderTarget &_target);

	//Returns the rect
	sf::IntRect const &GetRect() const {return m_ButtonSprite.GetRect();}

private:

	CSprite m_ButtonSprite;                       //the buttonsprite
	Buttontypes m_Buttontype;                       //the buttontype

	std::string m_Buttonstring;                  //the buttonstring
	sf::Text m_Buttontext; 

	float nextFrame;

	bool isEnabled;

	std::function<void()> m_Callback;

	void RenderButtontext(sf::RenderTarget &_target);

	void SetDefaultButtontext();

	bool ButtonWasPressed(CGameEngine &_engine);
};

