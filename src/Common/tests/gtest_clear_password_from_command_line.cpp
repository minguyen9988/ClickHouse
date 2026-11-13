#include <gtest/gtest.h>

#include <cstring>
#include <string>
#include <vector>
#include <Common/clearPasswordFromCommandLine.h>

namespace
{

/// Runs clearPasswordFromCommandLine over a copy of `args` and returns the resulting argv entries.
/// A cleared value becomes an empty string, because its bytes are all zero.
std::vector<std::string> clearArgs(std::vector<std::string> args)
{
    std::vector<std::vector<char>> storage;
    std::vector<char *> argv;
    for (const auto & arg : args)
    {
        storage.emplace_back(arg.begin(), arg.end());
        storage.back().push_back('\0');
    }
    for (auto & buffer : storage)
        argv.push_back(buffer.data());

    clearPasswordFromCommandLine(static_cast<int>(argv.size()), argv.data());

    std::vector<std::string> result;
    for (const char * arg : argv)
        result.emplace_back(arg);
    return result;
}

}

TEST(ClearPasswordFromCommandLine, PasswordSeparateValue)
{
    EXPECT_EQ(clearArgs({"clickhouse-client", "--password", "secret", "--query", "SELECT 1"}),
              (std::vector<std::string>{"clickhouse-client", "--password", "", "--query", "SELECT 1"}));
}

TEST(ClearPasswordFromCommandLine, PasswordEqualsValue)
{
    EXPECT_EQ(clearArgs({"clickhouse-client", "--password=secret", "--query", "SELECT 1"}),
              (std::vector<std::string>{"clickhouse-client", "--password=", "--query", "SELECT 1"}));
}

TEST(ClearPasswordFromCommandLine, JwtSeparateValue)
{
    EXPECT_EQ(clearArgs({"clickhouse-client", "--jwt", "header.payload.signature", "--query", "SELECT 1"}),
              (std::vector<std::string>{"clickhouse-client", "--jwt", "", "--query", "SELECT 1"}));
}

TEST(ClearPasswordFromCommandLine, JwtEqualsValue)
{
    EXPECT_EQ(clearArgs({"clickhouse-client", "--jwt=header.payload.signature", "--query", "SELECT 1"}),
              (std::vector<std::string>{"clickhouse-client", "--jwt=", "--query", "SELECT 1"}));
}

TEST(ClearPasswordFromCommandLine, BothOptions)
{
    EXPECT_EQ(clearArgs({"clickhouse-client", "--password", "p", "--jwt=t"}),
              (std::vector<std::string>{"clickhouse-client", "--password", "", "--jwt="}));
}

TEST(ClearPasswordFromCommandLine, OtherArgumentsUntouched)
{
    /// A value that merely starts with an option name, and similar option names, must stay as they are.
    const std::vector<std::string> untouched{"clickhouse-client", "--jwt-file", "path", "--passwordless", "--query", "--jwt-x=1"};
    EXPECT_EQ(clearArgs(untouched), untouched);
}

TEST(ClearPasswordFromCommandLine, OptionWithoutValueAtTheEnd)
{
    /// Nothing follows the option, so there is no value to clear and no out-of-bounds access.
    const std::vector<std::string> args{"clickhouse-client", "--jwt"};
    EXPECT_EQ(clearArgs(args), args);
}
