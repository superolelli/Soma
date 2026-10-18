#include "presentation/text/SkillDescription.hpp"

namespace soma::presentation
{
    std::string skillDescription(std::string_view title, std::string_view description)
    {
        std::string result{"*#ffa500 "};
        result += title;
        result += '*';
        if (!description.empty())
        {
            result += "\n#white ";
            result += description;
        }
        return result;
    }
} // namespace soma::presentation
