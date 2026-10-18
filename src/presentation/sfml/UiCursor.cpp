#include "presentation/sfml/UiCursor.hpp"

namespace soma::presentation
{
    void setUiCursor(sf::Window& window, bool hand)
    {
        struct Cursors
        {
            sf::Image normalImage{"Data/Cursors/brown/left_ptr.png"};
            sf::Image handImage{"Data/Cursors/brown/pointing_hand.png"};
            sf::Cursor normal{normalImage.getPixelsPtr(), normalImage.getSize(), {0U, 0U}};
            sf::Cursor hand{handImage.getPixelsPtr(), handImage.getSize(), {0U, 0U}};
        };
        static Cursors cursors;
        window.setMouseCursor(hand ? cursors.hand : cursors.normal);
    }
} // namespace soma::presentation
