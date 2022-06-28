#include <fstream>

#include "softeq/common/settings.hh"
#include "softeq/common/serialization/serializer.hh"
#include "softeq/common/serialization/json_serializer.hh"
#include "softeq/common/serialization/xml_serializer.hh"
#include "softeq/common/log.hh"

namespace softeq
{
namespace common
{
volatile const int __settingsAccessKey = 0;
}
} // namespace softeq

using namespace softeq::common;
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

    void serialize(Serializer &serializer, const std::map<std::string, Settings::mapValue_t> &node) const
    {
        serializer.beginObjectSerialization(Serializer::CurrentObjectType::STRUCT);
        for (auto &it : node)
        {
            if (it.second.assembler == nullptr)
            {
                throw std::logic_error("Assembler for settings '" + it.first + "' is not found");
            }
            serializer.name(it.first);
            it.second.assembler->serialize(serializer, it.second.value);
        }
        serializer.endObjectSerialization();
    }

    void deserialize(Serializer &serializer, std::map<std::string, Settings::mapValue_t> &node)
    {
        serializer.beginObjectDeserialization(Serializer::CurrentObjectType::STRUCT);
        const std::vector<std::string> keys = serializer.getNodeKeys();
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
            serializer.name(key);
            entry->second.assembler->deserialize(serializer, entry->second.value);
        }
        serializer.endObjectDeserialization();
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

void Settings::setDefaultSerializationParameters(const std::string &path, SerializationType type)
{
    _path = path;
    _type = type;
}

void Settings::serialize() const
{
    serialize(_path, _type);
}

void Settings::serialize(const std::string &path, SerializationType type) const
{
    if (path.empty())
    {
        throw std::logic_error("Path cannot be empty");
    }

    std::unique_ptr<Serializer> serializer;
    if (type == SerializationType::Json)
    {
        serializer.reset(new JSonSerializer());
    }
    else
    {
        serializer.reset(new XmlSerializer());
    }

    ObjectAssembler<decltype(_settings)>().serialize(*serializer, _settings);

    std::ofstream fileStream(path);
    fileStream << serializer->dump();
}

void Settings::deserialize()
{
    deserialize(_path);
}

void Settings::deserialize(const std::string &path)
{
    deserialize(path, _settings);
}

void Settings::deserialize(const std::string &path, std::map<std::string, mapValue_t> &settings)
{
    if (path.empty())
    {
        throw std::logic_error("Path cannot be empty");
    }

    std::ifstream fileStream(path);
    if (!fileStream.is_open())
    {
        // Not existing config for deserialization. Assume it is OK
        return;
    }
    std::string content;

    fileStream.seekg(0, std::ios::end);
    content.reserve(fileStream.tellg());
    fileStream.seekg(0, std::ios::beg);

    content.assign((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());

    // trim at beginning
    content.erase(content.begin(),
                  std::find_if(content.begin(), content.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    if (content.empty())
    {
        throw ParseException("Empty document for deserialization");
    }

    std::unique_ptr<Serializer> serializer;
    if (content[0] == '{')
    {
        serializer.reset(new JSonSerializer());
    }
    else if (content[0] == '<')
    {
        serializer.reset(new XmlSerializer());
    }
    else
    {
        throw ParseException("Not supported document type for deserialization");
    }

    serializer->setRawInput(content);

    ObjectAssembler<std::map<std::string, mapValue_t>>().deserialize(*serializer, settings);
}

std::string Settings::graph() const
{
    return ObjectAssembler<decltype(_settings)>().graph(_settings);
}
