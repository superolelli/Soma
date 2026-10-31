#include "persistence/SaveRepository.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <source_location>
#include <string_view>

namespace
{
    void check(bool condition, const std::source_location location = std::source_location::current())
    {
        if (!condition)
        {
            std::cerr << "Check failed at " << location.file_name() << ':' << location.line() << '\n';
            std::exit(EXIT_FAILURE);
        }
    }

    void checkStoredPlayer(const soma::persistence::SaveRepository& repository,
                           const std::filesystem::path& path,
                           std::string_view expectedName)
    {
        const auto loaded = repository.load(path);
        check(loaded.hasValue());
        check(loaded.value().playerName == expectedName);
    }

    void checkFailedReplacementPreservesOriginal(const soma::persistence::SaveRepository& repository,
                                                  const std::filesystem::path& savePath,
                                                  const std::filesystem::path& blocker,
                                                  bool blockerMustBeNonempty)
    {
        std::filesystem::create_directories(blocker);
        if (blockerMustBeNonempty)
        {
            std::ofstream marker{blocker / "keep-directory-nonempty"};
            marker << "block replacement";
        }

        soma::persistence::SaveGame changed;
        changed.playerName = "Changed";
        check(!repository.save(savePath, changed).hasValue());
        checkStoredPlayer(repository, savePath, "Original");

        std::error_code cleanupError;
        std::filesystem::remove_all(blocker, cleanupError);
        check(!cleanupError);
        check(repository.save(savePath, changed).hasValue());
        checkStoredPlayer(repository, savePath, "Changed");
    }
} // namespace

int main(int argc, char** argv)
{
    check(argc == 2);
    const auto testDirectory = std::filesystem::absolute(argv[1]);
    std::error_code cleanupError;
    std::filesystem::remove_all(testDirectory, cleanupError);
    check(!cleanupError);

    const soma::content::EconomyContent economy{};
    const soma::content::WorldContent world{};
    const soma::content::RuleContent rules{};
    const soma::persistence::SaveRepository repository{economy, world, rules};
    soma::persistence::SaveGame original;
    original.playerName = "Original";

    const auto temporaryFailurePath = testDirectory / "temporary-failure.json";
    check(repository.save(temporaryFailurePath, original).hasValue());
    checkFailedReplacementPreservesOriginal(
        repository, temporaryFailurePath, std::filesystem::path{temporaryFailurePath.string() + ".tmp"}, false);

    const auto backupFailurePath = testDirectory / "backup-failure.json";
    check(repository.save(backupFailurePath, original).hasValue());
    checkFailedReplacementPreservesOriginal(
        repository, backupFailurePath, std::filesystem::path{backupFailurePath.string() + ".bak"}, true);

    std::filesystem::remove_all(testDirectory, cleanupError);
    check(!cleanupError);
}
