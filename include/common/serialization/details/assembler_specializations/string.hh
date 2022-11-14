#ifndef SOFTEQ_COMMON_SERIALIZATION_STRING_OBJECT_ASSEMBLER_H
#define SOFTEQ_COMMON_SERIALIZATION_STRING_OBJECT_ASSEMBLER_H

template <typename T>
class ObjectAssembler<T, typename std::enable_if<std::is_same<T, std::string>::value>::type> final
{
public:
    inline static ObjectAssembler<std::string> accessor()
    {
        return ObjectAssembler<std::string>();
    }

    std::string graph(const std::string &assignedNodeName) const
    {
        return assignedNodeName + " [shape=note,label=<<I>string</I>>, height=0];";
    }

    void serializeElement(StructSerializer &serializer, const std::string &name, const std::string &node) const
    {
        serializer.serializeValue(name, node);
    }

    void serialize(ArraySerializer &serializer, const std::string &node) const
    {
        serializer.serializeValue(node);
    }

    void deserializeElement(StructDeserializer &deserializer, const std::string &name, std::string &node) const
    {
        node = softeq::common::stdutils::any_cast<std::string>(deserializer.value(name));
    }

    void deserialize(ArrayDeserializer &deserializer, std::string &node) const
    {
        node = softeq::common::stdutils::any_cast<std::string>(deserializer.value());
    }
};

#endif // SOFTEQ_COMMON_SERIALIZATION_STRING_OBJECT_ASSEMBLER_H
