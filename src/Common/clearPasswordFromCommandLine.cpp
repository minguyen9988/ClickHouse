#include <array>
#include <cstring>
#include <string_view>
#include <Common/clearPasswordFromCommandLine.h>

using namespace std::literals;

void clearPasswordFromCommandLine(int argc, char ** argv)
{
    /// Options whose value is a secret and must not be visible in `ps`.
    static constexpr std::array secret_options{"--password"sv, "--jwt"sv};

    for (int arg = 1; arg < argc; ++arg)
    {
        for (const auto option : secret_options)
        {
            std::string_view current(argv[arg]);

            if (arg + 1 < argc && current == option)
            {
                ++arg;
                memset(argv[arg], 0, strlen(argv[arg]));
                break;
            }

            if (current.size() > option.size() && current.starts_with(option) && current[option.size()] == '=')
            {
                char * value = argv[arg] + option.size() + 1;
                memset(value, 0, strlen(value));
                break;
            }
        }
    }
}
