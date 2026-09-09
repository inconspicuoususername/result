module;

#include <cctype>
#include <cstddef>
#include <format>
#include <functional>
#include <iostream>
#include <source_location>
#include <string>
#include <string_view>
#include <vector>
export module result:error;


namespace result {
    namespace detail {
        // Replace the contents of the outermost <...> / (...) pairs with "..."
        std::string collapse(std::string_view s, char open, char close) {
            std::string out;
            out.reserve(s.size());
            int depth = 0;
            bool inner = false;
            // saw non-space content inside the top-level pair
            for (const char c : s) {
                if (c == open) {
                    if (depth == 0) inner = false;
                    else inner = true; // a nested opener is content
                    ++depth;
                } else if (c == close && depth > 0) {
                    if (--depth == 0) {
                        out += open;
                        if (inner) out += "...";
                        out += close;
                    }
                } else if (depth == 0) {
                    out += c;
                } else if (c != ' ' && c != '\t') {
                    inner = true;
                }
            }
            return out;
        }

        // Drop GCC's module-ownership tags, e.g. "jm::Foo@engine.services" -> "jm::Foo".
        std::string strip_module_tags(std::string_view s) {
            std::string out;
            out.reserve(s.size());
            for (std::size_t i = 0; i < s.size(); ++i) {
                if (s[i] != '@') {
                    out += s[i];
                    continue;
                }
                ++i;
                while (i < s.size() &&
                       (std::isalnum(static_cast<unsigned char>(s[i])) || s[i]
                        == '_' || s[i] == '.'))
                    ++i;
                --i; // the for-loop's ++i lands us on the next real char
            }
            return out;
        }

        // "static Result<...> ns::Cls::method(a, b)" -> "ns::Cls::method(...)".
        std::string shorten_signature(std::string_view raw) {
            std::string s = strip_module_tags(raw);
            s = collapse(s, '<', '>');
            s = collapse(s, '(', ')');

            // Everything before the last '(' is "<return type> <qualified name>";
            // keep the last whitespace-separated token (the name) plus the params.
            const auto paren = s.find('(');
            const std::string_view sv{s};
            const std::string_view head = paren == std::string_view::npos
                                              ? sv
                                              : sv.substr(0, paren);
            const std::string_view tail = paren == std::string_view::npos
                                              ? std::string_view{}
                                              : sv.substr(paren);
            const auto sp = head.find_last_of(" \t");
            const std::string_view name = sp == std::string_view::npos
                                              ? head
                                              : head.substr(sp + 1);

            return std::string{name} + std::string{tail};
        }

        std::string_view basename(std::string_view path) {
            const auto pos = path.find_last_of("/\\");
            return pos == std::string_view::npos ? path : path.substr(pos + 1);
        }
    }

    export struct Error {
        struct Frame {
            std::string msg;
            std::source_location loc;
        };

        std::vector<Frame> frames;

        explicit Error(std::string msg,
                       std::source_location loc =
                           std::source_location::current())
            : frames{{std::move(msg), loc}} {
        }

        Error& context(std::string msg,
                       std::source_location loc =
                           std::source_location::current()) {
            frames.push_back({std::move(msg), loc});
            return *this;
        }

        std::string report() const;
    };

    // Out-of-line so it isn't inline and can freely use the detail:: helpers.
    std::string Error::report() const {
        std::size_t longest_fnc = 0;
        std::vector<std::pair<std::string, const Frame*> > refs(frames.size());
        for (int i = 0; i < frames.size(); i++) {
            auto& frame = frames[i];
            auto fnc_name =
                detail::shorten_signature(frame.loc.function_name());
            if (fnc_name.length() > longest_fnc)
                longest_fnc = fnc_name.length();
            refs[i] = std::pair(fnc_name, &frame);
        }

        // debug, TODO wrap in an ifdef
        auto [fnc_pretty, frame] = refs.front();
        if (frame->msg.empty()) {
            std::cerr <<
                "Recieved empty top frame message. This should not happen\n";
        }

        std::string out = std::format("{}", frame->msg);
        // out += "\n\nCaused by:";

        for (std::size_t i = 0; i < refs.size(); ++i) {
            const auto& [fnc_pretty, frame] = refs[i];

            std::string fmt = "\n    at {}";
            //add spaces
            for (auto j = 0; j < longest_fnc - fnc_pretty.size() + 2; j++)
                fmt += " ";
            fmt += "[{}:{}]";

            if (!frame->msg.empty() && i != 0) {
                fmt += "\n      └── {}";
            }
            out += std::format(
                std::dynamic_format(fmt),
                detail::shorten_signature(frame->loc.function_name()),
                detail::basename(frames[i].loc.file_name()),
                frames[i].loc.line(),
                frame->msg
            );
        }

        return out;
    }
}