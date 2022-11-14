#ifndef SOFTEQ_COMMON_SERIALIZATION_MAPS_OBJECT_ASSEMBLER_H
#define SOFTEQ_COMMON_SERIALIZATION_MAPS_OBJECT_ASSEMBLER_H

template <typename T>
struct isMap : std::false_type
{
};

template <typename K, typename V>
struct isMap<std::map<K, V>> : std::true_type
{
};

template <typename K, typename V>
struct isMap<std::unordered_map<K, V>> : std::true_type
{
};

template <typename T>
struct isStringKeyMap : std::false_type
{
};

template <typename T>
struct isStringKeyMap<std::map<std::string, T>> : std::true_type
{
};

template <typename T>
struct isStringKeyMap<std::unordered_map<std::string, T>> : std::true_type
{
};

template <typename T>
class ObjectAssembler<T, typename std::enable_if<isMap<T>::value && !isStringKeyMap<T>::value>::type> final
{
    static constexpr const char *cKeyName = "__mapKey__";
    static constexpr const char *cValueName = "__mapValue__";

public:
    inline static ObjectAssembler<T> accessor()
    {
        return ObjectAssembler<T>();
    }

    std::string graph(const std::string &assignedNodeName) const
    {
        std::string result =
            assignedNodeName +
            " [shape=plaintext, style=\"\", "
            "label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" BGCOLOR=\"lightgray\" CELLPADDING=\"3\">"
            "<TR><TD PORT=\"name\"><B>MAP</B></TD></TR>";
        result += "<TR><TD PORT=\"key\">key</TD></TR>";
        result += "<TR><TD PORT=\"value\">value</TD></TR>";
        result += "</TABLE>>];";

        std::string childKeyNodeName = assignedNodeName + "_mapKey";
        result += assignedNodeName + ":key -> " + childKeyNodeName + ";";
        result += ObjectAssembler<typename T::key_type>::accessor().graph(childKeyNodeName);

        std::string childValueNodeName = assignedNodeName + "_mapValue";
        result += assignedNodeName + ":value -> " + childValueNodeName + ";";
        result += ObjectAssembler<typename T::mapped_type>::accessor().graph(childValueNodeName);

        return result;
    }
    
    void serializeElement(StructSerializer &serializer, const std::string &name,
                          const T &node) const
    {
        ArraySerializer* mapElementsSerializer = serializer.serializeArray(name);
        assert(mapElementsSerializer);

        serializeMapAsArray(*mapElementsSerializer, node);
    }

    void serialize(ArraySerializer &serializer, const T &node) const
    {
        ArraySerializer* mapElementsSerializer = serializer.serializeArray();
        assert(mapElementsSerializer);

        serializeMapAsArray(*mapElementsSerializer, node);
    }

    void deserializeElement(StructDeserializer &deserializer, const std::string &name,
                            T &node) const
    {
        ArrayDeserializer* mapElementsDeserializer = deserializer.deserializeArray(name);
        assert(mapElementsDeserializer);

        deserializeMapFromArray(*mapElementsDeserializer, node);
    }

    void deserialize(ArrayDeserializer &deserializer, T &node) const
    {
        ArrayDeserializer* mapElementsDeserializer = deserializer.deserializeArray();
        assert(mapElementsDeserializer);

        deserializeMapFromArray(*mapElementsDeserializer, node);
    }

private:
    void serializeMapAsArray(ArraySerializer &serializer, const T &node) const
    {
        for (auto &it : node)
        {
            StructSerializer* elementSerializer = serializer.serializeStruct();
            ObjectAssembler<typename T::key_type>::accessor().serializeElement(
                    *elementSerializer, cKeyName, it.first);

            ObjectAssembler<typename T::mapped_type>::accessor().serializeElement(
                    *elementSerializer, cValueName, it.second);
        }
    }

    void deserializeMapFromArray(ArrayDeserializer &deserializer, T &node) const
    {
        using KeyType   = typename T::key_type;
        using ValueType = typename T::mapped_type;

        while(!deserializer.isComplete())
        {
            StructDeserializer* elementDeserializer = deserializer.deserializeStruct(); 

            KeyType deserializedKey;
            ValueType deserializedValue;

            ObjectAssembler<KeyType>::accessor().deserializeElement(
                *elementDeserializer, cKeyName, deserializedKey);
            ObjectAssembler<ValueType>::accessor().deserializeElement(
                *elementDeserializer, cValueName, deserializedValue);

            node[deserializedKey] = std::move(deserializedValue);
        }
    }
};

template <typename T>
class ObjectAssembler<T, typename std::enable_if<isStringKeyMap<T>::value>::type>
{
public:
    inline static ObjectAssembler<T> accessor()
    {
        return ObjectAssembler<T>();
    }

    std::string graph(const std::string &assignedNodeName) const
    {
        std::string result =
            assignedNodeName +
            " [shape=plaintext, style=\"\", "
            "label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" BGCOLOR=\"lightgray\" CELLPADDING=\"3\">"
            "<TR><TD PORT=\"name\"><B>HASHMAP</B></TD></TR>";
        result += "<TR><TD><I>string</I></TD></TR>";
        result += "<TR><TD PORT=\"value\">value</TD></TR>";
        result += "</TABLE>>];";

        std::string childValueNodeName = assignedNodeName + "_mapValue";
        result += assignedNodeName + ":value -> " + childValueNodeName + ":name;";
        result += ObjectAssembler<typename T::mapped_type>::accessor().graph(childValueNodeName);

        return result;
    }

    void serializeElement(StructSerializer &serializer, const std::string &name,
                          const T &node) const
    {
        ArraySerializer* mapElementsSerializer = serializer.serializeArray(name);
        assert(mapElementsSerializer);

        serializeMapAsArray(*mapElementsSerializer, node);
    }

    void serialize(ArraySerializer &serializer, const T &node) const
    {
        ArraySerializer* mapElementsSerializer = serializer.serializeArray();
        assert(mapElementsSerializer);

        serializeMapAsArray(*mapElementsSerializer, node);
    }

    void deserializeElement(StructDeserializer &deserializer, const std::string &name,
                            T &node) const
    {
        ArrayDeserializer* mapElementsDeserializer = deserializer.deserializeArray(name);
        assert(mapElementsDeserializer);

        deserializeMapFromArray(*mapElementsDeserializer, node);
    }

    void deserialize(ArrayDeserializer &deserializer, T &node) const
    {
        ArrayDeserializer* mapElementsDeserializer = deserializer.deserializeArray();
        assert(mapElementsDeserializer);

        deserializeMapFromArray(*mapElementsDeserializer, node);
    }

private:
    void serializeMapAsArray(ArraySerializer &serializer, const T &node) const
    {
        for (auto &it : node)
        {
            StructSerializer* elementSerializer = serializer.serializeStruct();
            assert(elementSerializer);

            ObjectAssembler<typename T::mapped_type>::accessor().serializeElement(
                    *elementSerializer, it.first, it.second);
        }
    }

    void deserializeMapFromArray(ArrayDeserializer &deserializer, T &node) const
    {
        node.clear();
        while(!deserializer.isComplete())
        {
            StructDeserializer* elementDeserializer = deserializer.deserializeStruct(); 
            assert(elementDeserializer);

            std::vector<std::string> storedKeys = elementDeserializer->availableNames();
            std::for_each(storedKeys.cbegin(), storedKeys.cend(),
                          [&elementDeserializer, &node](const std::string &key)
                          {
                              typename T::mapped_type deserializedValue;
                              ObjectAssembler<typename T::mapped_type>::accessor()
                                  .deserializeElement(*elementDeserializer, key,
                                                      deserializedValue);
                              node[key] = std::move(deserializedValue);
                          });
        }
    }
};

#endif //SOFTEQ_COMMON_SERIALIZATION_MAPS_OBJECT_ASSEMBLER_H
