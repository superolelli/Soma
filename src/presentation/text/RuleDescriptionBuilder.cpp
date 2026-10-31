#include "presentation/text/RuleDescriptionBuilder.hpp"

namespace soma::presentation
{
    namespace
    {
        std::string substituteSubject(std::string value, const std::string& subject)
        {
            if (const auto position = value.find("{subject}"); position != std::string::npos)
                value.replace(position, std::string{"{subject}"}.size(), subject);
            return value;
        }

        std::string capitalized(std::string value)
        {
            if (!value.empty() && value.front() >= 'a' && value.front() <= 'z') value.front() -= 'a' - 'A';
            return value;
        }
    }

    std::string buildGermanRuleDescription(const gameplay::ComposedRule& rule,
                                           std::optional<content::RuleMode> mode,
                                           const content::RuleContent& content)
    {
        if (mode == content::RuleMode::triggered && rule.trigger && rule.subject && rule.effect)
        {
            const auto& trigger = content.fragments.at(*rule.trigger).sentenceTemplate;
            const auto& subjectDefinition = content.fragments.at(*rule.subject);
            const auto& subject = subjectDefinition.sentenceTemplate;
            const auto plural = std::get<content::SubjectFragment>(subjectDefinition.payload).plural;
            const auto& effectDefinition = content.fragments.at(*rule.effect);
            const auto& effectTemplate = plural ? effectDefinition.pluralSentenceTemplate : effectDefinition.sentenceTemplate;
            const auto effect = substituteSubject(effectTemplate, subject);
            return "Wenn " + trigger + ", " + effect + ".";
        }
        if (mode == content::RuleMode::standing && rule.subject && rule.modifier)
        {
            const auto& subject = content.fragments.at(*rule.subject).sentenceTemplate;
            return capitalized(substituteSubject(content.fragments.at(*rule.modifier).sentenceTemplate, subject)) + ".";
        }
        return "Unvollständige Regel";
    }

    std::optional<content::RuleMode> completedRuleMode(const gameplay::ComposedRule& rule)
    {
        if (rule.trigger && rule.subject && rule.effect && !rule.modifier)
        {
            return content::RuleMode::triggered;
        }
        if (!rule.trigger && rule.subject && !rule.effect && rule.modifier)
        {
            return content::RuleMode::standing;
        }
        return std::nullopt;
    }

    std::vector<std::string> buildGermanActiveRuleDescriptions(const gameplay::ComposedRules& rules,
                                                                const content::RuleContent& content)
    {
        std::vector<std::string> result;
        for (const auto& rule : rules)
        {
            const auto mode = completedRuleMode(rule);
            if (mode)
            {
                result.push_back(buildGermanRuleDescription(rule, mode, content));
            }
        }
        return result;
    }
}
