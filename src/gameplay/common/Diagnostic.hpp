#pragma once
#include <filesystem>
#include <string>

namespace soma::gameplay
{
    enum class DiagnosticSeverity
    {
        warning,
        error
    };

    struct Diagnostic
    {
        DiagnosticSeverity severity{DiagnosticSeverity::error};
        std::string code;
        std::filesystem::path file;
        std::string field;
        std::string message;
    };
} // namespace soma::gameplay
