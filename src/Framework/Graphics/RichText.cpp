#include "RichText.hpp"
#include <SFML/Graphics.hpp>
#include <sstream>

#ifndef NULL
#define NULL NULL
#endif

namespace
{
	struct chunk
	{
		chunk()
		{
			style = sf::Text::Regular;
			color = sf::Color::White;
			endsInNewline = false;
		}

		sf::String
		text;

		int
		style;

		sf::Color
		color;

		bool
		endsInNewline;
	};

	void newChunk(std::vector<chunk>& chunks, chunk*& currentChunk, chunk& lastChunk)
	{
		chunk newChunk;

		//	only carry information over if currentChunk and lastChunk aren't the same
		//	this only happens for the first chunk, but it causes random errors
		if (chunks.size() > 1)
		{
			newChunk.style = lastChunk.style;
			newChunk.color = lastChunk.color;
		}

		chunks.push_back(newChunk);
		currentChunk = &(chunks[chunks.size() - 1]);
	}

	void processFormatting(chunk& lastChunk, chunk* currentChunk, sf::Text::Style style)
	{
		if ((lastChunk.style & style) >= 0)
		{
			currentChunk->style ^= style;
		}
		else
		{
			currentChunk->style |= style;
		}
	}

}

namespace sfe
{
	RichText::RichText() 
		: characterSize(30)
		, font(NULL)
		, centered(false) 
		, outlineThickness(0.0f)
		, outlineColor(sf::Color::Black)
	{
		initializeColors();
	}

	RichText::RichText(const sf::String& source, const sf::Font& font, unsigned int characterSize) 
		: characterSize(characterSize)
		, font(&font)
		, centered(false)
		, outlineThickness(0.0f)
		, outlineColor(sf::Color::Black)
	{
		initializeColors();
		setString(source);
	}

	sf::String RichText::getString() const
	{
		return string;
	}

	sf::String RichText::getSource() const
	{
		return source;
	}

	void RichText::setString(const sf::String& source)
	{
		this->source = source;

		if (font == NULL)
		{
			return;
		}

		clear();

		std::vector<chunk> chunks;
		chunks.push_back(chunk());

		chunk* currentChunk = &(chunks[0]);
		bool escaped = false;

		for (unsigned int i = 0; i < source.getSize(); ++i)
		{
			chunk& lastChunk = *currentChunk;

			switch (source[i])
			{
				case '~': //	italics
				{
					if (escaped)
					{
						currentChunk->text += source[i];
						escaped = false;
						break;
					}

					newChunk(chunks, currentChunk, lastChunk);

					processFormatting(lastChunk, currentChunk, sf::Text::Italic);

					currentChunk->color = lastChunk.color;
					break;
				}
				case '*': //	bold
				{
					if (escaped)
					{
						currentChunk->text += source[i];
						escaped = false;
						break;
					}

					newChunk(chunks, currentChunk, lastChunk);

					processFormatting(lastChunk, currentChunk, sf::Text::Bold);

					currentChunk->color = lastChunk.color;
					break;
				}
				case '_': 	//	underline
				{
					if (escaped)
					{
						currentChunk->text += source[i];
						escaped = false;
						break;
					}

					newChunk(chunks, currentChunk, lastChunk);

					processFormatting(lastChunk, currentChunk, sf::Text::Underlined);

					currentChunk->color = lastChunk.color;
					break;
				}
				case '#':	//	color
				{
					if (escaped)
					{
						currentChunk->text += source[i];
						escaped = false;
						break;
					}

					int length = 0;
					int start = i + 1;

					//	seek forward until the next whitespace
					while (!isspace(source[++i]))
					{
						++length;
					}

					newChunk(chunks, currentChunk, lastChunk);
					currentChunk->color = getColor(source.toWideString().substr(start, length));;
					break;

				}
				case '\\':	//	escape sequence for escaping formatting characters
				{
					if (i < source.getSize())
					{
						switch (source[i + 1])
						{
							case '~':
							case '*':
							case '_':
							case '#':
							{
								escaped = true;
								break;
							}
							default:
								break;
						}
					}

					if (!escaped)
					{
						currentChunk->text += source[i];
					}

					break;
				}
				case '\n':	// make a new chunk in the case of a newline
				{
					currentChunk->endsInNewline = true;
					newChunk(chunks, currentChunk, lastChunk);
					break;
				}
				default:
				{
					escaped = false;
					currentChunk->text += source[i];
					break;
				}
			}
		}

		string = "";
		for (unsigned int i = 0; i < chunks.size(); ++i)
		{
			if (!chunks[i].endsInNewline && chunks[i].text.getSize() == 0)
			{
				continue;
			}

			string += chunks[i].text + (chunks[i].endsInNewline ? "\n" : "");
		}

		sf::Text text;
		text.setString(string);
		text.setCharacterSize(characterSize);

		if (font != NULL)
		{
			text.setFont(*font);
		}

		bounds = text.getLocalBounds();

		int cursor = 0;
		chunk* lastChunk = NULL;
		for (unsigned int i = 0; i < chunks.size(); ++i)
		{
			sf::Text t;
			t.setFillColor(chunks[i].color);
			t.setString(chunks[i].text);
			t.setStyle(chunks[i].style);
			t.setCharacterSize(characterSize);
			t.setOutlineColor(outlineColor);
			t.setOutlineThickness(outlineThickness);

			if (font != NULL)
			{
				t.setFont(*font);
			}

			t.setPosition(text.findCharacterPos(cursor));

			if (lastChunk != NULL && lastChunk->endsInNewline)
			{
				t.setPosition(0, t.getPosition().y + font->getLineSpacing(characterSize));
				++cursor;
			}

			texts.push_back(t);
			cursor += chunks[i].text.getSize();
			lastChunk = &chunks[i];
		}


		if (centered)
			center();
	}

	void RichText::clear()
	{
		texts.clear();
		bounds = sf::FloatRect();
	}

	unsigned int RichText::getCharacterSize() const
	{
		return characterSize;
	}

	void RichText::setCharacterSize(unsigned int size)
	{
		characterSize = std::max(size, 1u);

		setString(source);
	}

	const sf::Font* RichText::getFont() const
	{
		return font;
	}

	void RichText::setFont(const sf::Font& font)
	{
		this->font = &font;

		setString(source);
	}

	void RichText::setOutlineColor(sf::Color _color)
	{
		outlineColor = _color;
	}

	void RichText::setOutlineThickness(float _thickness)
	{
		outlineThickness = _thickness;
	}


	void RichText::setCentered(bool _centered)
	{
		centered = _centered;
	}

	sf::FloatRect RichText::getLocalBounds() const
	{
		return bounds;
	}


	sf::FloatRect RichText::getGlobalBounds() const
	{
		return getTransform().transformRect(getLocalBounds());
	}

	void
	RichText::addColor(const sf::String& name, const sf::Color& color)
	{
		colors[name] = color;
	}

	void
	RichText::addColor(const sf::String& name, unsigned int argbHex)
	{
		colors[name] = getColor(argbHex);
	}

	void RichText::draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		states.transform *= getTransform();

		// Draw
		for(unsigned int i = 0; i < texts.size(); ++i)
		{
			// Draw text
			target.draw(texts[i], states);
		}
	}


	void RichText::center()
	{
		//get longest line length
		int lastY = texts[0].getPosition().y;
		int longestLength = 0;
		int currentLength = 0;
		int numOfTextInLine = 0;
		std::vector<int> lineLengths;
		std::vector<int> numOfTextsInLine;
		for (int i = 0; i < texts.size(); i++)
		{
			if (texts[i].getPosition().y != lastY)
			{
				lastY = texts[i].getPosition().y;
				if (currentLength > longestLength)
					longestLength = currentLength;

				lineLengths.push_back(currentLength);
				numOfTextsInLine.push_back(numOfTextInLine);
				currentLength = 0;
				numOfTextInLine = 0;
			}

			currentLength += texts[i].getLocalBounds().width;
			numOfTextInLine++;
		}

		if (currentLength > longestLength)
			longestLength = currentLength;
		lineLengths.push_back(currentLength);
		numOfTextsInLine.push_back(numOfTextInLine);


		float positionX = 0.0f;
		int text = 0;
		//set position of lines
		for (int i = 0; i < lineLengths.size(); i++)
		{
			positionX = longestLength / 2.0f - lineLengths[i] / 2.0f;
			for (int j = 0; j < numOfTextsInLine[i]; j++)
			{
				texts[text].setPosition(texts[text].getPosition().x + positionX, texts[text].getPosition().y);
				text++;
			}
			
		}
	}


	void
	RichText::initializeColors()
	{
		colors["default"] = sf::Color::White;
		colors["black"] = sf::Color::Black;
		colors["blue"] = sf::Color::Blue;
		colors["cyan"] = sf::Color::Cyan;
		colors["green"] = sf::Color::Green;
		colors["magenta"] = sf::Color::Magenta;
		colors["red"] = sf::Color::Red;
		colors["white"] = sf::Color::White;
		colors["yellow"] = sf::Color::Yellow;
	}

	sf::Color
	RichText::getColor(const sf::String& source) const
	{
		std::map<sf::String, sf::Color>::const_iterator result = colors.find(source);
		if (result == colors.end())
		{
			unsigned int hex = 0x0;
			if (!(std::istringstream(source) >> std::hex >> hex))
			{
				//	Error parsing; return default
				return sf::Color::White;
			};

			return getColor(hex);
		}

		return result->second;
	}

	sf::Color
	RichText::getColor(unsigned int argbHex) const
	{
		argbHex |= 0xff000000;
		return sf::Color(argbHex >> 16 & 0xFF, argbHex >> 8 & 0xFF, argbHex >> 0 & 0xFF, argbHex >> 24 & 0xFF);
	}

}
