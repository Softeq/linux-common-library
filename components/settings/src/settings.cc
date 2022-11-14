#include "settings.hh"

#include <common/logging/log.hh>

#include <fstream>
#include <algorithm>
#include <stdexcept>

namespace softeq
{
namespace common
{
namespace settings
{
volatile const int __settingsAccessKey = 0;
}
} // namespace common
} // namespace softeq

namespace
{
std::string extractFileContent(std::ifstream &inputStream)
{
    std::string content;

    inputStream.seekg(0, std::ios::end);
    content.reserve(inputStream.tellg());
    inputStream.seekg(0, std::ios::beg);

    content.assign((std::istreambuf_iterator<char>(inputStream)), std::istreambuf_iterator<char>());

    // trim at beginning
    content.erase(content.begin(),
                  std::find_if(content.begin(), content.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));

    return content;
}

} // namespace

using namespace softeq::common::settings;
using namespace softeq::common::serialization;

template <>
class softeq::common::serialization::ObjectAssembler<std::map<std::string, Settings::mapValue_t>>
{
public:
    inline static ObjectAssembler<std::map<std::string, Settings::mapValue_t>> accessor()
    {
        return ObjectAssembler<std::map<std::string, Settings::mapValue_t>>();
    }

    std::string graph(const std::map<std::string, Settings::mapValue_t> &settings) const
    {
        std::string result = "digraph struct {\nrankdir=LR; bgcolor=\"#ffffff00\";\n";
        result +=
            "node [fontname=\"sans\", fontsize=\"10\", fillcolor=\"lightgray\", style=\"filled\", margin=\"0.1\"];\n";

        std::string assignedNodeName = "root";

        result += assignedNodeName + " [shape=record,label=\"{{";
        size_t index = 0;
        for (auto &it : settings)
        {
            result += "<member" + std::to_string(index) + ">";
            result += it.first;
            index++;

            if (index != settings.size())
            {
                result += "|";
            }
        }
        result += "}}\", height=0.1];\n";

        index = 0;
        for (auto &it : settings)
        {
            std::string childNodeName = assignedNodeName + "_" + it.first;
            std::string lineLength;
            result += assignedNodeName + ":member" + std::to_string(index) + " -> " + childNodeName + ":name;\n";

            if (it.second.assembler == nullptr)
            {
                throw std::logic_error("Assembler for settings '" + it.first + "' is not found");
            }
            result += it.second.assembler->graph(childNodeName);

            index++;
        }

        result += "}";
        return result;
    }

    void serialize(StructSerializer &serializer, const std::map<std::string, Settings::mapValue_t> &node) const
    {
        for (auto &it : node)
        {
            if (it.second.assembler == nullptr)
            {
                throw std::logic_error("Assembler for settings '" + it.first + "' is not found");
            }
            it.second.assembler->serialize(serializer, it.first, it.second.value);
        }
    }

    void deserialize(StructDeserializer &deserializer, std::map<std::string, Settings::mapValue_t> &node)
    {
        const std::vector<std::string> keys = deserializer.availableNames();
        for (const std::string &key : keys)
        {
            auto entry = node.find(key);
            if (entry == node.end())
            {
                // type is not registered yet or name has been changed
                continue;
            }

            if (entry->second.assembler == nullptr)
            {
                throw std::logic_error("Assembler for settings '" + entry->first + "' is not found");
            }
            entry->second.assembler->deserialize(deserializer, entry->first, entry->second.value);
        }
    }
};

Settings::Settings()
{
}

Settings &Settings::instance()
{
    static Settings settings;
    return settings;
}

void Settings::setDefaultParameters(const std::string &path,
                                    std::unique_ptr<StructDeserializer> &&deserializer,
                                    std::unique_ptr<StructSerializer> &&serializer)
{
    _path = path;
    _serializer = std::move(serializer);
    _deserializer = std::move(deserializer);
}

void Settings::serialize() const
{
    if(!_path.empty() && _serializer)
    {
        serialize(_path, *_serializer);
    }
    else
    {
        throw std::invalid_argument("Default serialization parameters are not set.");
    }
}

void Settings::deserialize()
{
    if(!_path.empty() && _deserializer)
    {
        deserialize(_path, *_deserializer);
    }
    else
    {
        throw std::invalid_argument("Default deserialization parameters are not set.");
    }
}

void Settings::serialize(const std::string &path, StructSerializer &serializer) const
{
    ObjectAssembler<decltype(_settings)>::accessor().serialize(serializer, _settings);
    std::ofstream fileStream(path);
    if(!fileStream.is_open())
    {
        throw std::invalid_argument("Wrong file path for serialization.");
    }
    fileStream << serializer.dump();
}

void Settings::deserialize(const std::string &path, StructDeserializer &deserializer)
{
    std::ifstream fileStream(path);
    if(!fileStream.is_open())
    {
        throw std::invalid_argument("Wrong file path for deserialization.");
    }
    deserialize(fileStream, deserializer, _settings);
}

void Settings::deserialize(std::ifstream &settingsFileStream, StructDeserializer &deserializer,
                           std::map<std::string, mapValue_t> &settings)
{
    if (settingsFileStream.is_open())
    {
        std::string content = extractFileContent(settingsFileStream);

        if (content.empty())
        {
            throw std::logic_error("Empty document for deserialization");
        }

        deserializer.setRawInput(content);
        ObjectAssembler<std::map<std::string, mapValue_t>>().deserialize(deserializer, settings);
    }
}

std::string Settings::graph() const
{
    return ObjectAssembler<decltype(_settings)>().graph(_settings);
}
