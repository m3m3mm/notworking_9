#include "stat_reader.h"
#include <iostream>
#include <iomanip>

namespace transport_catalogue {
namespace output {

void ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view request,
                       std::ostream& output) {
    auto space_pos = request.find(' ');
    if (space_pos == request.npos) {
        output << "Invalid request format" << std::endl;
        return;
    }

    std::string_view command = request.substr(0, space_pos);
    std::string_view name = request.substr(space_pos + 1);
    name = name.substr(name.find_first_not_of(" "));  // Trim leading spaces
    name = name.substr(0, name.find_last_not_of(" ") + 1);  // Trim trailing spaces

    if (command == "Bus") {
        const auto bus_info = transport_catalogue.GetBusInfo(std::string(name));
        if (bus_info.stops_count == 0) {
            output << "Bus " << name << ": not found" << std::endl;
        } else {
            output << "Bus " << name << ": " << bus_info.stops_count << " stops on route, "
                   << bus_info.unique_stops_count << " unique stops, "
                   << std::setprecision(6) << std::fixed << bus_info.route_length << " route length, "
                   << std::setprecision(6) << std::fixed << bus_info.curvature << " curvature" << std::endl;
        }
    } else if (command == "Stop") {
        if (!transport_catalogue.HasStop(std::string(name))) {
            output << "Stop " << name << ": not found" << std::endl;
        } else {
            const auto& buses = transport_catalogue.GetBusesForStop(std::string(name));
            if (buses.empty()) {
                output << "Stop " << name << ": no buses" << std::endl;
            } else {
                output << "Stop " << name << ": buses";
                for (const auto& bus : buses) {
                    output << " " << bus;
                }
                output << std::endl;
            }
        }
    } else {
        output << "Unknown command: " << command << std::endl;
    }
}

} // namespace output
} // namespace transport_catalogue
