#ifndef SOFTEQ_COMMON_SERIALIZATION_LIST_OR_VECTOR_OBJECT_ASSEMBLER_H
#define SOFTEQ_COMMON_SERIALIZATION_LIST_OR_VECTOR_OBJECT_ASSEMBLER_H

template <typename T>
struct isStdVector : std::false_type
{
};
template <typename T>
struct isStdVector<std::vector<T>> : std::true_type
{
};

template <typename T>
struct isStdList : std::false_type
{
};
template <typename T>
struct isStdList<std::list<T>> : std::true_type
{
};

template <typename T>
class ObjectAssembler<T, typename std::enable_if<isStdList<T>::value || isStdVector<T>::value>::type> final
{
public:
    inline static ObjectAssembler<T> accessor()
    {
        return ObjectAssembler<T>();
    }

    std::string graph(const std::string &assignedNodeName) const
    {
        std::string result;
        result += "subgraph cluster_array_" + assignedNodeName + "{rank = same;  bgcolor=\"#ffffffff\";";
        result += assignedNodeName + " [shape=plaintext, style=\"\", label=<<B>ARRAY</B>>];";
        std::string childNodeName = assignedNodeName + "_" + "array";
        result += assignedNodeName + " -> " + childNodeName + ":name;";
        result += ObjectAssembler<typename T::value_type>::accessor().graph(childNodeName);
        result += "};";
        return result;
    }

    void serializeElement(StructSerializer &serializer, const std::string &name, const T &node) const
    {
        ArraySerializer *arraySerializer = serializer.serializeArray(name);
        assert(arraySerializer);
        serializeArray(*arraySerializer, node);
    }

    void serialize(ArraySerializer &serializer, const T &node) const
    {
        ArraySerializer *arraySerializer = serializer.serializeArray();
        assert(arraySerializer);
        serializeArray(*arraySerializer, node);
    }

    void deserializeElement(StructDeserializer &deserializer, const std::string &name, T &node) const
    {
        ArrayDeserializer *arrayDeserializer = deserializer.deserializeArray(name);

        // TODO: need to reconsider all asserts
        if (!arrayDeserializer)
        {
            throw ParseException(name, "Looks like mandatory node is null");
        }
        deserializeArray(*arrayDeserializer, node);
    }

    void deserialize(ArrayDeserializer &deserializer, T &node) const
    {
        ArrayDeserializer *arrayDeserializer = deserializer.deserializeArray();
        // TODO: test for array in array when inner array is null
        assert(arrayDeserializer);
        deserializeArray(*arrayDeserializer, node);
    }

private:
    void serializeArray(ArraySerializer &serializer, const T &node) const
    {
        for (auto &it : node)
        {
            ObjectAssembler<typename T::value_type>::accessor().serialize(serializer, it);
        }
    }

    void deserializeArray(ArrayDeserializer &deserializer, T &node) const
    {
        node.clear();
        while (!deserializer.isComplete())
        {
            node.push_back({});
            try
            {
                ObjectAssembler<typename T::value_type>::accessor().deserialize(deserializer, node.back());
            }
            catch (const std::exception &ex)
            {
                throw ParseException(std::to_string(deserializer.index()), "Looks like mandatory element is null");
            }
        }
    }
};

#endif // SOFTEQ_COMMON_SERIALIZATION_LIST_OR_VECTOR_OBJECT_ASSEMBLER_H
