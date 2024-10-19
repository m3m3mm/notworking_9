
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include <deque>
#include <unordered_set>
#include <string_view>
#include "geo.h"

namespace transport_catalogue {

struct Stop {
    std::string name;
    geo::Coordinates coordinates;
};

struct Bus {
    std::string name;
    std::vector<const Stop*> stops;
    bool is_roundtrip;
};

struct BusInfo {
    int stops_count;
    int unique_stops_count;
    double route_length;
    double curvature;
};

class TransportCatalogue {
public:
    void AddStop(std::string_view name, geo::Coordinates coordinates);
    void AddBus(std::string_view name, const std::vector<std::string>& stops, bool is_roundtrip);
    void AddStopDistance(std::string_view from_stop_name, std::string_view to_stop_name, int distance);
    const Bus* FindBus(std::string_view name) const;
    const Stop* FindStop(std::string_view name) const;
    BusInfo GetBusInfo(std::string_view name) const;
    std::vector<std::string> GetBusesForStop(std::string_view stop_name) const;
    bool HasStop(std::string_view stop_name) const;

private:
    struct StopPair {
        const Stop* from;
        const Stop* to;

        bool operator==(const StopPair& other) const {
            return from == other.from && to == other.to;
        }
    };

    struct StopPairHasher {
        std::size_t operator()(const StopPair& pair) const {
            const std::size_t h1 = std::hash<const Stop*>{}(pair.from);
            const std::size_t h2 = std::hash<const Stop*>{}(pair.to);
            return h1 ^ (h2 << 1);
        }
    };

    std::deque<Stop> stops_;
    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
    std::unordered_map<std::string_view, const Bus*> busname_to_bus_;
    std::unordered_map<const Stop*, std::set<std::string>> stop_to_buses_;
    std::unordered_map<StopPair, int, StopPairHasher> distances_;
};

} // namespace transport_catalogue
