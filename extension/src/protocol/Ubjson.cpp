#include "protocol/Ubjson.h"

#include <cstring>

#if defined(_MSC_VER)
#include <stdlib.h>
#endif

namespace presonus::studiolive::gpext::protocol
{

namespace
{

std::uint32_t readUInt32Be(std::span<const std::uint8_t> bytes)
{
    std::uint32_t value = 0;
    std::memcpy(&value, bytes.data(), sizeof(value));
#if defined(_MSC_VER)
    value = _byteswap_ulong(value);
#elif defined(__GNUC__) || defined(__clang__)
    value = __builtin_bswap32(value);
#endif
    return value;
}

float readFloatBe(std::span<const std::uint8_t> bytes)
{
    const std::uint32_t bits = readUInt32Be(bytes);
    float value = 0.0F;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

std::int32_t readInt32Be(std::span<const std::uint8_t> bytes)
{
    std::int32_t value = 0;
    std::memcpy(&value, bytes.data(), sizeof(value));
#if defined(_MSC_VER)
    value = static_cast<std::int32_t>(_byteswap_ulong(static_cast<std::uint32_t>(value)));
#elif defined(__GNUC__) || defined(__clang__)
    value = __builtin_bswap32(value);
#endif
    return value;
}

std::int64_t readInt64Be(std::span<const std::uint8_t> bytes)
{
    std::int64_t value = 0;
    std::memcpy(&value, bytes.data(), sizeof(value));
#if defined(_MSC_VER)
    value = _byteswap_uint64(value);
#elif defined(__GNUC__) || defined(__clang__)
    value = __builtin_bswap64(value);
#endif
    return value;
}

struct Frame
{
    enum class Kind
    {
        Object,
        Array,
    };
    Kind kind;
    UbjsonObject *object{nullptr};
    UbjsonArray *array{nullptr};
    std::string pendingKey;
};

std::optional<UbjsonValue> readTypedValue(std::span<const std::uint8_t> data, std::size_t &index,
                                          std::uint8_t type)
{
    std::size_t length = 0;
    switch (type)
    {
    case 0x53:
        if (index >= data.size() || data[index++] != 0x69)
        {
            return std::nullopt;
        }
        length = data[index++];
        break;
    case 0x64:
        length = 4;
        break;
    case 0x69:
    case 0x55:
        length = 1;
        break;
    case 0x6C:
        length = 4;
        break;
    case 0x4C:
        length = 8;
        break;
    default:
        return std::nullopt;
    }

    if (index + length > data.size())
    {
        return std::nullopt;
    }

    const auto raw = data.subspan(index, length);
    index += length;

    switch (type)
    {
    case 0x53:
        return UbjsonValue(
            std::string(reinterpret_cast<const char *>(raw.data()), raw.size()));
    case 0x64:
        return UbjsonValue(static_cast<double>(readFloatBe(raw)));
    case 0x69:
        return UbjsonValue(static_cast<std::int64_t>(static_cast<std::int8_t>(raw[0])));
    case 0x55:
        return UbjsonValue(static_cast<std::int64_t>(raw[0]));
    case 0x6C:
        return UbjsonValue(static_cast<std::int64_t>(readInt32Be(raw)));
    case 0x4C:
        return UbjsonValue(readInt64Be(raw));
    default:
        return std::nullopt;
    }
}

void assignValue(Frame &parent, UbjsonValue value)
{
    if (parent.kind == Frame::Kind::Array)
    {
        parent.array->emplace_back(std::move(value));
        return;
    }
    parent.object->emplace(parent.pendingKey, std::move(value));
}

} // namespace

UbjsonValue::UbjsonValue(Storage value) : storage_(std::move(value))
{
}

bool UbjsonValue::isObject() const { return storage_.index() == 6; }
bool UbjsonValue::isArray() const { return storage_.index() == 5; }
bool UbjsonValue::isString() const { return storage_.index() == 4; }
bool UbjsonValue::isNumber() const
{
    return storage_.index() == 2 || storage_.index() == 3;
}

const UbjsonObject *UbjsonValue::asObject() const
{
    if (!isObject())
    {
        return nullptr;
    }
    return &std::get<UbjsonObject>(storage_);
}

const UbjsonArray *UbjsonValue::asArray() const
{
    if (!isArray())
    {
        return nullptr;
    }
    return &std::get<UbjsonArray>(storage_);
}

std::optional<std::string> UbjsonValue::asString() const
{
    if (!isString())
    {
        return std::nullopt;
    }
    return std::get<std::string>(storage_);
}

std::optional<double> UbjsonValue::asNumber() const
{
    if (storage_.index() == 2)
    {
        return static_cast<double>(std::get<std::int64_t>(storage_));
    }
    if (storage_.index() == 3)
    {
        return std::get<double>(storage_);
    }
    return std::nullopt;
}

std::optional<UbjsonObject> parseUbjsonObject(std::span<const std::uint8_t> data)
{
    if (data.empty() || data[0] != 0x7B)
    {
        return std::nullopt;
    }

    UbjsonObject root;
    std::vector<Frame> stack;
    stack.push_back(Frame{.kind = Frame::Kind::Object, .object = &root});

    std::size_t index = 1;
    while (index < data.size() && !stack.empty())
    {
        Frame &current = stack.back();

        if (current.kind == Frame::Kind::Array)
        {
            if (data[index] == 0x5D)
            {
                ++index;
                stack.pop_back();
                continue;
            }
        }
        else if (data[index] == 0x7D)
        {
            ++index;
            stack.pop_back();
            continue;
        }

        std::optional<std::string> key;
        if (current.kind == Frame::Kind::Object)
        {
            if (data[index++] != 0x69)
            {
                return std::nullopt;
            }
            const std::uint8_t keyLength = data[index++];
            if (index + keyLength > data.size())
            {
                return std::nullopt;
            }
            key = std::string(reinterpret_cast<const char *>(data.data() + index), keyLength);
            index += keyLength;
            current.pendingKey = *key;
        }

        const std::uint8_t type = data[index++];
        if (type == 0x7B)
        {
            UbjsonObject child;
            if (current.kind == Frame::Kind::Array)
            {
                current.array->emplace_back(UbjsonValue(child));
                auto &placed = std::get<UbjsonObject>(current.array->back().storageMut());
                stack.push_back(Frame{.kind = Frame::Kind::Object, .object = &placed});
            }
            else
            {
                auto [it, _] = current.object->emplace(current.pendingKey, UbjsonValue(child));
                auto &placed = std::get<UbjsonObject>(it->second.storageMut());
                stack.push_back(Frame{.kind = Frame::Kind::Object, .object = &placed});
            }
            continue;
        }
        if (type == 0x5B)
        {
            UbjsonArray child;
            if (current.kind == Frame::Kind::Array)
            {
                current.array->emplace_back(UbjsonValue(child));
                auto &placed = std::get<UbjsonArray>(current.array->back().storageMut());
                stack.push_back(Frame{.kind = Frame::Kind::Array, .array = &placed});
            }
            else
            {
                auto [it, _] = current.object->emplace(current.pendingKey, UbjsonValue(child));
                auto &placed = std::get<UbjsonArray>(it->second.storageMut());
                stack.push_back(Frame{.kind = Frame::Kind::Array, .array = &placed});
            }
            continue;
        }

        auto value = readTypedValue(data, index, type);
        if (!value)
        {
            return std::nullopt;
        }
        assignValue(current, std::move(*value));
    }

    if (!stack.empty())
    {
        return std::nullopt;
    }
    return root;
}

} // namespace presonus::studiolive::gpext::protocol
