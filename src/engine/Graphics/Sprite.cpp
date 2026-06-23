#include "Sprite.hpp"


CSprite::CSprite(sf::Texture const& _texture)
	: m_Sprite(_texture)
	, m_NumFrames(0)
	, m_FrameHeight(0)
	, m_FrameWidth(0)
	, m_NumFramesX(0)
{
	//Set the rectangle
	m_Rect.position.y = 0;
	m_Rect.position.x = 0;
	m_Rect.size.y = m_Sprite.getTextureRect().size.y;
	m_Rect.size.x = m_Sprite.getTextureRect().size.x;
}

CSprite::CSprite(sf::Texture const& _texture, int _numFrames, int _frameWidth, int _frameHeight)
	: CSprite(_texture)
{
	//set the frame rect
	m_NumFrames = _numFrames;
	m_FrameHeight = _frameHeight;
	m_FrameWidth = _frameWidth;
	m_FrameRect.size.y = _frameHeight;
	m_FrameRect.size.x = _frameWidth;
	m_NumFramesX = m_Rect.size.x / m_FrameWidth;

	m_Rect.size.y = _frameHeight;
	m_Rect.size.x = _frameWidth;
}

CSprite::~CSprite()
{

}

void CSprite::SetPos(int _x, int _y)
{
	//set the rect position
	m_Rect.position.x = _x;
	m_Rect.position.y = _y;

	//set the "real" position
	m_Sprite.setPosition(sf::Vector2f(static_cast<float>(_x), static_cast<float>(_y)));
}



void CSprite::SetPos(float _x, float _y)
{
	//set the rect position
	m_Rect.position.x = static_cast<int>(_x);
	m_Rect.position.y = static_cast<int>(_y);

	//set the real position
	m_Sprite.setPosition(sf::Vector2f(_x, _y));
}


void CSprite::Move(float _x, float _y)
{
	m_Sprite.move(sf::Vector2f(_x, _y));

	m_Rect.position.x = (int)m_Sprite.getPosition().x;
	m_Rect.position.y = (int)m_Sprite.getPosition().y;
}


//sets the texture rect
void CSprite::SetTextureRect(sf::IntRect const &_rect)
{
	m_Sprite.setTextureRect(_rect);
}


void CSprite::SetTextureRectToDefault()
{
	m_Sprite.setTextureRect(sf::IntRect({0, 0}, sf::Vector2i(m_Sprite.getTexture().getSize())));
}


void CSprite::SetOrigin(float _x, float _y)
{
	m_Sprite.setOrigin(sf::Vector2f(_x, _y));
}


void CSprite::Rotate(float _angle)
{
	m_Sprite.rotate(sf::degrees(_angle));
}


void CSprite::SetRotation(float _angle)
{
	m_Sprite.setRotation(sf::degrees(_angle));
}


void CSprite::ResetRotation()
{
	m_Sprite.setRotation(sf::degrees(0.0f));
}


void CSprite::SetScale(float _x, float _y)
{
	m_Sprite.setScale(sf::Vector2f(_x, _y));

	m_Rect.size.x = m_Sprite.getLocalBounds().size.x;
	m_Rect.size.y = m_Sprite.getLocalBounds().size.y;
}

//render "normal" sprites
void CSprite::Render(sf::RenderTarget &_target, sf::IntRect *clip)
{
	//check if a clip was given
	if (clip != nullptr)
	{
		m_Sprite.setTextureRect(*clip);
		_target.draw(m_Sprite);
	}
	else
		_target.draw(m_Sprite);
}


//render animated sprites
void CSprite::Render(sf::RenderTarget &_target, float _fFrameNumber, bool _picturewise)
{
	if (_picturewise)
	{
		//calculate column
		auto Column = static_cast<int>(_fFrameNumber) % m_NumFramesX;

		//calculate row
		auto Row = static_cast<int>(_fFrameNumber) / m_NumFramesX;

		//calculate rect
		m_FrameRect.position.x = Column* m_FrameWidth;
		m_FrameRect.position.y = Row * m_FrameHeight;
	}
	else
	{
		//for sprites, that are not rendered picturewise
		m_FrameRect.position.x = m_FrameWidth - (int)(_fFrameNumber * m_FrameWidth);
		m_FrameRect.position.y = 0;
	}

	//render Sprite
	Render(_target, &m_FrameRect);

}



void CSprite::SetColor(int _r, int _g, int _b, int _a)
{
	m_Sprite.setColor(sf::Color(_r, _g, _b, _a));
}

const sf::Color &CSprite::GetColor()
{
	return m_Sprite.getColor();
}

sf::IntRect CSprite::GetGlobalRect() const
{
	const auto bounds = m_Sprite.getGlobalBounds();
	return {{static_cast<int>(bounds.position.x), static_cast<int>(bounds.position.y)},
		{static_cast<int>(bounds.size.x), static_cast<int>(bounds.size.y)}};
}

