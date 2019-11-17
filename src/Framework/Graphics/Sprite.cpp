#include "Sprite.hpp"




CSprite::CSprite()
{
	m_NumFrames = 0;
}




CSprite::~CSprite()
{

}




void CSprite::Load(sf::Texture const &_texture)
{
	//Set the texture
	m_Sprite.setTexture(_texture);

	//Check, if the texture was set
	if (m_Sprite.getTexture() == NULL)
	{
		std::cout << "Couldn't apply texture" << std::endl;
	}

	//Set the rectangle
	m_Rect.top = 0;
	m_Rect.left = 0;
	m_Rect.height = m_Sprite.getTextureRect().height;
	m_Rect.width = m_Sprite.getTextureRect().width;
}



void CSprite::Load(sf::Texture const &_texture, int _numFrames, int _frameWidth, int _frameHeight)
{
	//Set the texture
	Load(_texture);

	//set the frame rect
	m_NumFrames = _numFrames;
	m_FrameHeight = _frameHeight;
	m_FrameWidth = _frameWidth;
	m_FrameRect.height = _frameHeight;
	m_FrameRect.width = _frameWidth;
	m_NumFramesX = m_Rect.width / m_FrameWidth;

	m_Rect.height = _frameHeight;
	m_Rect.width = _frameWidth;
}



void CSprite::SetPos(int _x, int _y)
{
	//set the rect position
	m_Rect.left = _x;
	m_Rect.top = _y;

	//set the "real" position
	m_Sprite.setPosition(static_cast<float>(_x), static_cast<float>(_y));
}



void CSprite::SetPos(float _x, float _y)
{
	//set the rect position
	m_Rect.left = static_cast<int>(_x);
	m_Rect.top = static_cast<int>(_y);

	//set the real position
	m_Sprite.setPosition(_x, _y);
}


void CSprite::Move(float _x, float _y)
{
	m_Sprite.move(_x, _y);

	m_Rect.left = (int)m_Sprite.getPosition().x;
	m_Rect.top = (int)m_Sprite.getPosition().y;
}


//sets the texture rect
void CSprite::SetTextureRect(sf::IntRect const &_rect)
{
	m_Sprite.setTextureRect(_rect);
}


void CSprite::SetTextureRectToDefault()
{
	m_Sprite.setTextureRect(sf::IntRect(0, 0, m_Sprite.getTexture()->getSize().x, m_Sprite.getTexture()->getSize().y));
}


void CSprite::SetOrigin(float _x, float _y)
{
	m_Sprite.setOrigin(_x, _y);
}


void CSprite::Rotate(float _angle)
{
	m_Sprite.rotate(_angle);
}


void CSprite::SetRotation(float _angle)
{
	m_Sprite.setRotation(_angle);
}


void CSprite::ResetRotation()
{
	m_Sprite.setRotation(0.0f);
}


void CSprite::SetScale(float _x, float _y)
{

	m_Rect.width = static_cast<int>(m_Rect.width * _x);
	m_Rect.height = static_cast<int>(m_Rect.height * _y);

	m_Sprite.setScale(_x, _y);
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
		m_FrameRect.left = Column* m_FrameWidth;
		m_FrameRect.top = Row * m_FrameHeight;
	}
	else
	{
		//for sprites, that are not rendered picturewise
		m_FrameRect.left = m_FrameWidth - (int)(_fFrameNumber * m_FrameWidth);
		m_FrameRect.top = 0;
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
	return (sf::IntRect)m_Sprite.getGlobalBounds();
}

