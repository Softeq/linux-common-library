#ifndef SOFTEQ_COMMON_SERIALIZATION_ENUM_OBJECT_ASSEMBLER_H
#define SOFTEQ_COMMON_SERIALIZATION_ENUM_OBJECT_ASSEMBLER_H

template <typename T>
class ObjectAssembler<T, typename std::enable_if<std::is_enum<T>::value>::type> final
{
    typedef std::map<std::string, T> ValueMap;
    ValueMap _map;

public:
    using Self = ObjectAssembler<T>;

    static ObjectAssembler<T> accessor()
    {
        return Assembler<T>();
    }

    Self &define(const std::string &name, T v)
    {
        if (_map.find(name) != _map.end())
        {
            throw std::logic_error("redeclaration of enum with name '" + name + "'");
        }

        _map[name] = v;
        return *this;
    }

    void serializeElement(StructSerializer &serializer, const std::string &name,
                          const T &node) const
    {
        serializer.serializeValue(name, findEnumValueName(node));
    }

    void serialize(ArraySerializer &serializer, const T &node) const
    {
        serializer.serializeValue(findEnumValueName(node));
    }

    void deserializeElement(StructDeserializer &deserializer, const std::string &name,
                            T &node) const
    {
        assignEnumValueByName(deserializer.value(name), node);
    }

    void deserialize(ArrayDeserializer &deserializer, T &node) const
    {
        assignEnumValueByName(deserializer.value(), node);
    }

    std::string graph(const std::string &assignedNodeName) const
    {
        if (_map.empty())
        {
            throw std::runtime_error(std::string("The enum map (") +
                                     softeq::common::stdutils::demangle(typeid(T).name()) +
                                     ") must have at least one value defined.");
        }

        std::string result =
            assignedNodeName +
            " [shape=plaintext, style=\"\" ,"
            "label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" BGCOLOR=\"lightgray\" CELLPADDING=\"3\">"
            "<TR><TD PORT=\"name\"><B>ENUM</B></TD></TR>";
        result += "<TR><TD>";
        for (typename ValueMap::const_iterator it = _map.begin(); it != _map.end(); ++it)
        {
            if (it != _map.begin())
            {
                result += "<BR/>";
            }
            result += it->first;
        }
        result += "</TD></TR>";
        result += "</TABLE>>];";
        return result;
    }

private:
    std::string valueAsString(const T &v) const
    {
        return std::to_string(static_cast<typename std::underlying_type<T>::type>(v));
    }

    std::string findEnumValueName(const T &node) const
    {
        auto enumNameIt =
            std::find_if(_map.cbegin(), _map.cend(),
                         [node](const typename ValueMap::value_type &enumNameRecord)
                         {
                             return enumNameRecord.second == node;
                         });
        if (enumNameIt != _map.end())
        {
            return (*enumNameIt).first;
        }
        else
        {
            throw std::runtime_error(
                    std::string("No mapped value ")
                    + valueAsString(node) + " specified for type "
                    + softeq::common::stdutils::demangle(typeid(T).name()));
        }
    }

    void assignEnumValueByName(const softeq::common::stdutils::Any &deserializedName,
                               T &deserializedValue) const
    {
        std::string value =
            softeq::common::stdutils::any_cast<std::string>(deserializedName);

        typename ValueMap::const_iterator it = _map.find(value);
        if (it != _map.end())
        {
            deserializedValue = it->second;
        }
        else
        {
            throw std::runtime_error("Unknown enumeration value");
        }
    }

};

#endif //SOFTEQ_COMMON_SERIALIZATION_ENUM_OBJECT_ASSEMBLER_H
