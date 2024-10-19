#include "transport_catalogue.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace transport_catalogue {

void TransportCatalogue::AddStop(std::string_view name, geo::Coordinates coordinates) {
    stops_.push_back({std::string(name), coordinates});
    const Stop* stop_ptr = &stops_.back();
    stopname_to_stop_[stops_.back().name] = stop_ptr; // сохраняем полную строку для последующего использования
}

void TransportCatalogue::AddStopDistance(std::string_view from_stop_name, std::string_view to_stop_name, int distance) {
    const Stop* from_stop = FindStop(from_stop_name);
    const Stop* to_stop = FindStop(to_stop_name);
    if (from_stop && to_stop) {
        distances_[{from_stop, to_stop}] = distance;
    }
}

const Bus* TransportCatalogue::FindBus(std::string_view name) const {
    auto it = busname_to_bus_.find(name);
    return it != busname_to_bus_.end() ? it->second : nullptr;
}

const Stop* TransportCatalogue::FindStop(std::string_view name) const {
    auto it = stopname_to_stop_.find(name);
    return it != stopname_to_stop_.end() ? it->second : nullptr;
}

std::vector<std::string> TransportCatalogue::GetBusesForStop(std::string_view stop_name) const {
    const Stop* stop = FindStop(stop_name);
    if (!stop) return {};

    auto it = stop_to_buses_.find(stop);
    if (it != stop_to_buses_.end()) {
        return std::vector<std::string>(it->second.begin(), it->second.end());
    }
    return {};
}

BusInfo TransportCatalogue::GetBusInfo(std::string_view name) const {
    const Bus* bus = FindBus(name);
    if (!bus) {
        return {0, 0, 0.0, 0.0};
    }

    int stops_count = bus->is_roundtrip ? bus->stops.size() : bus->stops.size() * 2 - 1;
    std::unordered_set<const Stop*> unique_stops(bus->stops.begin(), bus->stops.end());
    int unique_stops_count = unique_stops.size();

    double route_length = 0.0;
    double geo_route_length = 0.0;

    auto compute_distance = [this](const Stop* from, const Stop* to) {
        auto it = distances_.find({from, to});
        if (it != distances_.end()) {
            return it->second;
        }
        it = distances_.find({to, from});
        if (it != distances_.end()) {
            return it->second;  // Use the reverse distance if available
        }
        return static_cast<int>(geo::ComputeDistance(from->coordinates, to->coordinates));
    };

    for (size_t i = 1; i < bus->stops.size(); ++i) {
        const Stop* from = bus->stops[i - 1];
        const Stop* to = bus->stops[i];

        route_length += compute_distance(from, to);
        geo_route_length += geo::ComputeDistance(from->coordinates, to->coordinates);
    }

    if (!bus->is_roundtrip && bus->stops.size() > 1) {
        for (size_t i = bus->stops.size() - 1; i > 0; --i) {
            const Stop* from = bus->stops[i];
            const Stop* to = bus->stops[i - 1];

            route_length += compute_distance(from, to);
            geo_route_length += geo::ComputeDistance(from->coordinates, to->coordinates);
        }
    }

    double curvature = route_length / geo_route_length;

    return {stops_count, unique_stops_count, route_length, curvature};
}

void TransportCatalogue::AddBus(std::string_view name, const std::vector<std::string>& stops, bool is_roundtrip) {
    buses_.push_back({std::string(name), {}, is_roundtrip});
    Bus& bus = buses_.back();
    for (const auto& stop_name : stops) {
        const Stop* stop = FindStop(stop_name);
        if (stop) {
            bus.stops.push_back(stop);
            stop_to_buses_[stop].insert(std::string(name));
        }
    }
    busname_to_bus_[std::string(name)] = &bus;
}

bool TransportCatalogue::HasStop(std::string_view stop_name) const {
    return stopname_to_stop_.find(std::string(stop_name)) != stopname_to_stop_.end();
}
    
} // namespace transport_catalogue
