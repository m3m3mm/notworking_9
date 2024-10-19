

#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include "geo.h"
#include "transport_catalogue.h"

namespace transport_catalogue {
namespace input {

struct CommandDescription {
    explicit operator bool() const {
        return !command.empty();
    }

    bool operator!() const {
        return !operator bool();
    }

    std::string command;
    std::string id;
    std::string description;
};

geo::Coordinates ParseCoordinates(std::string_view str);
std::vector<std::string> ParseRoute(std::string_view route);


class InputReader {
public:
    void ParseLine(std::string_view line);
    void ApplyCommands(TransportCatalogue& catalogue) const;

private:
    std::vector<CommandDescription> commands_;
};

} // namespace input
} // namespace transport_catalogue
