#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace utils {

namespace {
    class args_helper {
    public:
        args_helper(std::string help_page)
            : help_page_ { help_page }
        {
        }

        struct positional_arg {
            std::string& write_to;
        };

        struct optional_arg {
            std::string& write_to;

            std::string symbol;
        };

        struct boolean_arg {
            bool& write_to;

            std::string symbol;
        };

    private:
        std::string help_page_;
        std::vector<positional_arg> positionals_;
        std::vector<optional_arg> optionals_;
        std::vector<boolean_arg> booleans_;

    public:
        auto help_page() const -> std::string
        {
            return help_page_;
        }

        auto set_positional(positional_arg arg) -> positional_arg&
        {
            positionals_.push_back(arg);
            return positionals_.back();
        }

        auto set_optional(optional_arg arg) -> optional_arg&
        {
            optionals_.push_back(arg);
            return optionals_.back();
        }

        auto set_boolean(boolean_arg arg) -> boolean_arg&
        {
            booleans_.push_back(arg);
            return booleans_.back();
        }

        auto parse(int argc, char** argv) const -> bool
        {
            std::size_t i {};
            while (i + 1 < static_cast<std::size_t>(argc) && i < positionals_.size()) {
                positionals_[i].write_to = std::string { argv[i + 1] };

                ++i;
            }

            if (i < positionals_.size()) {
                return false;
            }

            for (;i + 1 < static_cast<std::size_t>(argc); ++i) {
                std::string arg { argv[i + 1] };

                auto found_optional = std::find_if(optionals_.begin(), optionals_.end(),
                    [&arg](const optional_arg& opt) {
                        return opt.symbol == arg;
                    });

                if (found_optional != optionals_.end()) {
                    ++i;
                    if (!(i + 1 < static_cast<std::size_t>(argc))) {
                        throw std::runtime_error { "no arg value: " + arg };
                    }

                    std::string option { argv[i + 1] };
                    found_optional->write_to = option;
                    continue;
                }

                auto found_boolean = std::find_if(booleans_.begin(), booleans_.end(),
                    [&arg](const boolean_arg& opt) {
                        return opt.symbol == arg;
                    });
                
                if (found_boolean != booleans_.end()) {
                    found_boolean->write_to = true;
                    continue;
                }

                    throw std::runtime_error { "unknown arg: " + arg };

            }

            return true;
        }
    };
}
}
