// input_reader.cpp

#include "input_reader.h"
#include <algorithm>
#include <cassert>
#include <iterator>
#include <string>
#include <vector>

namespace transport_catalogue {
namespace input {
namespace detail {

std::vector<std::string> SplitString(std::string_view str, std::string_view delimiter) {
    std::vector<std::string> result;
    size_t pos = 0;
    while ((pos = str.find(delimiter)) != std::string_view::npos) {
        result.push_back(std::string(str.substr(0, pos)));
        str.remove_prefix(pos + delimiter.size());
    }
    result.push_back(std::string(str));  // Добавляем последний элемент
    return result;
}

std::unordered_map<std::string, int> ParseDistances(std::string_view str) {
    std::unordered_map<std::string, int> distances;
    
    // Разделяем строку по разделителю ", "
    auto parts = SplitString(str, ", ");
    
    // Проходим по частям, начиная с третьего элемента
    for (size_t i = 2; i < parts.size(); ++i) {
        // Разделяем строку вида "3900m to Marushkino" по "m to "
        auto distance_parts = SplitString(parts[i], "m to ");
        if (distance_parts.size() == 2) {
            int distance = std::stoi(distance_parts[0]);  // Парсим расстояние
            std::string stop_name = distance_parts[1];  // Получаем имя остановки
            distances[stop_name] = distance;
        }
    }
    
    return distances;
}


std::vector<std::string> ParseRoute(std::string_view route) {
    std::vector<std::string> stops;
    size_t pos = 0;
    while (pos < route.length()) {
        size_t next = route.find_first_of(">-", pos);
        if (next == std::string_view::npos) {
            next = route.length();
        }
        std::string_view stop = route.substr(pos, next - pos);
        stop = stop.substr(stop.find_first_not_of(' '));
        stop = stop.substr(0, stop.find_last_not_of(' ') + 1);
        stops.push_back(std::string(stop));
        pos = next + 1;
    }
    return stops;
}

} // namespace detail

void InputReader::ParseLine(std::string_view line) {
    size_t colon = line.find(':');
    if (colon == line.npos) {
        return;
    }

    std::string_view command_part = line.substr(0, colon);
    size_t space = command_part.find(' ');
    if (space == command_part.npos) {
        return;
    }

    commands_.push_back({
        std::string(command_part.substr(0, space)),
        std::string(command_part.substr(space + 1)),
        std::string(line.substr(colon + 2))
    });
}

void InputReader::ApplyCommands(TransportCatalogue& catalogue) const {
    // Первый проход: добавляем остановки
    for (const auto& command : commands_) {
        if (command.command == "Stop") {
            // Используем SplitString для разбивки строки на координаты и прочее
            auto parts = detail::SplitString(command.description, ", ");
            if (parts.size() >= 2) {
                double lat = std::stod(parts[0]);
                double lng = std::stod(parts[1]);
                catalogue.AddStop(command.id, {lat, lng});
            }
        }
    }
    
    // Второй проход: добавляем расстояния и автобусы
    for (const auto& command : commands_) {
        if (command.command == "Stop") {
            auto distances = detail::ParseDistances(command.description);
            for (const auto& [stop_name, distance] : distances) {
                catalogue.AddStopDistance(command.id, stop_name, distance);  // Три аргумента: начальная остановка, конечная остановка, расстояние
            }
        } else if (command.command == "Bus") {
            auto stops = detail::ParseRoute(command.description);
            bool is_roundtrip = command.description.find('>') != std::string::npos;
            catalogue.AddBus(command.id, stops, is_roundtrip);
        }
    }
}


} // namespace input
} // namespace transport_catalogue
