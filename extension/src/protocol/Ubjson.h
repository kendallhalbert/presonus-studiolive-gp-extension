#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <span>
#include <string>
#include <variant>
#include <vector>

namespace presonus::studiolive::gpext::protocol
{

using UbjsonObject = std::map<std::string, class UbjsonValue>;
using UbjsonArray = std::vector<class UbjsonValue>;

class UbjsonValue
{
  public:
    using Storage =
        std::variant<std::monostate, bool, std::int64_t, double, std::string, UbjsonArray, UbjsonObject>;

    UbjsonValue() = default;
    explicit UbjsonValue(Storage value);

    bool isObject() const;
    bool isArray() const;
    bool isString() const;
    bool isNumber() const; // int64 or double

    const UbjsonObject *asObject() const;
    const UbjsonArray *asArray() const;
    std::optional<std::string> asString() const;
    std::optional<double> asNumber() const;

    const Storage &storage() const { return storage_; }
    Storage &storageMut() { return storage_; }

  private:
    Storage storage_;
};

/// Parse a UBJSON object root `{...}` (partial UCNet dialect).
std::optional<UbjsonObject> parseUbjsonObject(std::span<const std::uint8_t> data);

} // namespace presonus::studiolive::gpext::protocol
