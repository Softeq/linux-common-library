#ifndef SOFTEQ_COMMON_SERIALIZATION_OPTIONAL_OBJECT_ASSEMBLER_H
#define SOFTEQ_COMMON_SERIALIZATION_OPTIONAL_OBJECT_ASSEMBLER_H

template <typename T>
class ObjectAssembler<softeq::common::stdutils::Optional<T>> final
{
public:
    inline static ObjectAssembler<softeq::common::stdutils::Optional<T>> accessor()
    {
        return ObjectAssembler<softeq::common::stdutils::Optional<T>>();
    }

    std::string graph(const std::string &assignedNodeName) const
    {
        std::string result;
        result += "subgraph cluster_optional_" + assignedNodeName + "{rank = same;  bgcolor=\"#ffffffff\";";
        result += assignedNodeName + " [shape=plaintext, style=\"\", label=<<B>OPTIONAL</B>>];";
        std::string childNodeName = assignedNodeName + "_opt";
        result += assignedNodeName + " -> " + childNodeName + ":name;";
        result += ObjectAssembler<T>::accessor().graph(childNodeName);
        result += "};";

        return result;
    }

    void serializeElement(StructSerializer &serializer, const std::string &name,
                          const softeq::common::stdutils::Optional<T> &node) const
    {
        if (node.hasValue())
        {
            ObjectAssembler<T>::accessor().serializeElement(serializer, name, node.cValue());
        }
    }

    void serialize(ArraySerializer &serializer, const softeq::common::stdutils::Optional<T> &node) const
    {
        if (node.hasValue())
        {
            ObjectAssembler<T>::accessor().serialize(serializer, node.cValue());
        }
        else
        {
            serializer.serializeEmpty();
        }
    }

    void deserializeElement(StructDeserializer &deserializer, const std::string &name,
                            softeq::common::stdutils::Optional<T> &node) const
    {
        if (deserializer.valueExists(name))
        {
            // optional should never throw an exception. in case of error it must be not initialized
            try
            {
                T value;
                ObjectAssembler<T>::accessor().deserializeElement(deserializer, name, value);
                node = softeq::common::stdutils::Optional<T>(value);
            }
            // TODO enumerate correct exceptions bad_cast out_of_range
            catch (...)
            {
                node = softeq::common::stdutils::Optional<T>();
            }
        }
        else
        {
            node = softeq::common::stdutils::Optional<T>();
        }
    }

    void deserialize(ArrayDeserializer &deserializer, softeq::common::stdutils::Optional<T> &node) const
    {
        if (deserializer.nextValueExists())
        {
            // optional should never throw an exception. in case of error it must be not initialized
            try
            {
                T value;
                ObjectAssembler<T>::accessor().deserialize(deserializer, value);
                node = softeq::common::stdutils::Optional<T>(value);
            }
            // TODO enumerate correct exceptions bad_cast out_of_range
            catch (...)
            {
                deserializer.value(); // Skip null value
                node = softeq::common::stdutils::Optional<T>();
            }
        }
        else
        {
            deserializer.value(); // Skip null value
            node = softeq::common::stdutils::Optional<T>();
        }
    }
};

#endif // SOFTEQ_COMMON_SERIALIZATION_OPTIONAL_OBJECT_ASSEMBLER_H
