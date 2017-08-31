#pragma once


#include "../Gameengine.hpp"
#include "../Graphics/Sprite.hpp"

enum class Buttontypes{Up, Down, Motion_Up, Motion_Down};

class CButton
{
public:

	CButton();
    ~CButton();

	//Loads the button, defines the type and sets the position
	void Load(sf::Texture const &_texture, Buttontypes _type, std::string const &_str = "");

	//Sets the button's position
	void SetPos(int _x, int _y);

	//sets the scale of the button
	void SetScale(float _x, float _y);

	void SetButtontextFont(sf::Font const &_font);
	void SetButtontextCharactersize(int _size);
	void SetButtontextColor(sf::Color const &_color);
	void SetButtontextStyle(sf::Text::Style _style);

	//sets the buttonstring
	void SetButtonstring(std::string const &_buttonstring);

	bool Update(CGameEngine &_engine);

	//renders the button, returns true if button was pressed
	void Render(CGameEngine &_engine);

	//Returns the rect
	sf::IntRect const &GetRect() const {return m_ButtonSprite.GetRect();}

private:

	CSprite m_ButtonSprite;                       //the buttonsprite
	Buttontypes m_Buttontype;                       //the buttontype

	std::string m_Buttonstring;                  //the buttonstring
	sf::Text m_Buttontext; 

	float nextFrame;

	void RenderButtontext(CGameEngine &_engine);

	void SetDefaultButtontext();
};

