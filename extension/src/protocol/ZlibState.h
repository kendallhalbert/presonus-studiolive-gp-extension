#pragma once

#include "protocol/Ubjson.h"

#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace presonus::studiolive::gpext::protocol
{

using StateValue = std::variant<std::monostate, bool, std::int64_t, double, std::string>;

struct ZlibStateNode
{
    std::optional<StateValue> value;
    std::unordered_map<std::string, ZlibStateNode> children;
};

/// Parse inflated UBJSON synchronize payload into a navigable tree.
std::optional<ZlibStateNode> parseZlibState(const UbjsonObject &root);

/// Query slash paths (e.g. `global/mixer_name` on Synchronize, `internal/global/...` on full state).
const ZlibStateNode *zlibStateFind(const ZlibStateNode &root, std::string_view path);

std::optional<StateValue> zlibStateValueAt(const ZlibStateNode &root, std::string_view path);

std::optional<std::string> zlibStateStringAt(const ZlibStateNode &root, std::string_view path);

} // namespace presonus::studiolive::gpext::protocol
