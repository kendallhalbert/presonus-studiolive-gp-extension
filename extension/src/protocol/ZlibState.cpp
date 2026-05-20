#include "protocol/ZlibState.h"

namespace presonus::studiolive::gpext::protocol
{

namespace
{

StateValue ubjsonToStateValue(const UbjsonValue &value)
{
    if (value.isString())
    {
        return *value.asString();
    }
    if (const auto number = value.asNumber())
    {
        return *number;
    }
    return std::monostate{};
}

ZlibStateNode parseInputNode(const UbjsonObject &node, std::vector<std::string> path);

void mergeChildren(ZlibStateNode &target, const UbjsonObject &children,
                   const std::vector<std::string> &path)
{
    for (const auto &[key, child] : children)
    {
        if (const auto *childObject = child.asObject())
        {
            std::vector<std::string> next = path;
            next.emplace_back(key);
            target.children.emplace(key, parseInputNode(*childObject, next));
        }
    }
}

void mergeValues(ZlibStateNode &target, const UbjsonObject &values)
{
    for (const auto &[key, value] : values)
    {
        auto &child = target.children[key];
        child.value = ubjsonToStateValue(value);
    }
}

ZlibStateNode parseInputNode(const UbjsonObject &node, std::vector<std::string> path)
{
    ZlibStateNode result;
    for (const auto &[key, entry] : node)
    {
        if (key == "children")
        {
            if (const auto *children = entry.asObject())
            {
                mergeChildren(result, *children, path);
            }
        }
        else if (key == "values")
        {
            if (const auto *values = entry.asObject())
            {
                mergeValues(result, *values);
            }
        }
    }
    return result;
}

const ZlibStateNode *walk(const ZlibStateNode &node, std::string_view segment)
{
    const auto it = node.children.find(std::string(segment));
    if (it == node.children.end())
    {
        return nullptr;
    }
    return &it->second;
}

} // namespace

std::optional<ZlibStateNode> parseZlibState(const UbjsonObject &root)
{
    if (!root.contains("children"))
    {
        return std::nullopt;
    }

    return parseInputNode(root, {});
}

const ZlibStateNode *zlibStateFind(const ZlibStateNode &root, std::string_view path)
{
    const ZlibStateNode *current = &root;
    std::size_t start = 0;
    while (start < path.size())
    {
        const std::size_t slash = path.find('/', start);
        const std::string_view segment =
            path.substr(start, slash == std::string_view::npos ? std::string_view::npos : slash - start);
        if (!segment.empty())
        {
            current = walk(*current, segment);
            if (current == nullptr)
            {
                return nullptr;
            }
        }
        if (slash == std::string_view::npos)
        {
            break;
        }
        start = slash + 1;
    }
    return current;
}

std::optional<StateValue> zlibStateValueAt(const ZlibStateNode &root, std::string_view path)
{
    const ZlibStateNode *node = zlibStateFind(root, path);
    if (node == nullptr || !node->value)
    {
        return std::nullopt;
    }
    return *node->value;
}

std::optional<std::string> zlibStateStringAt(const ZlibStateNode &root, std::string_view path)
{
    const auto value = zlibStateValueAt(root, path);
    if (!value || !std::holds_alternative<std::string>(*value))
    {
        return std::nullopt;
    }
    return std::get<std::string>(*value);
}

} // namespace presonus::studiolive::gpext::protocol
