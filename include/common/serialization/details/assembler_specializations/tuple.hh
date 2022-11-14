#ifndef SOFTEQ_COMMON_SERIALIZATION_TUPLE_OBJECT_ASSEMBLER_H
#define SOFTEQ_COMMON_SERIALIZATION_TUPLE_OBJECT_ASSEMBLER_H

template <template<class...> typename T, typename...Types>
class ObjectAssembler<T<Types...>,
                      typename std::enable_if<
                                   std::is_same<T<Types...>,
                                       std::tuple<Types...>>::value>::type> final
{
public:
    inline static ObjectAssembler<T<Types...>> accessor()
    {
        return ObjectAssembler<T<Types...>>();
    }

    std::string graph(const std::string &assignedNodeName) const
    {
        return assignedNodeName + " [shape=note,label=<<I>string</I>>, height=0];";
    }

    void serializeElement(StructSerializer &serializer, const std::string &name,
                          const std::tuple<Types...> &node) const
    {
        ArraySerializer* tupleSerializer = serializer.serializeArray(name);
        assert(tupleSerializer);
        serializeTupleElements(*tupleSerializer, node);
    }

    void serialize(ArraySerializer &serializer, const std::tuple<Types...> &node) const
    {
        ArraySerializer* tupleSerializer = serializer.serializeArray();
        assert(tupleSerializer);
        serializeTupleElements(*tupleSerializer, node);
    }

    void deserializeElement(StructDeserializer &deserializer, const std::string &name,
                            std::tuple<Types...> &node) const
    {
        ArrayDeserializer* tupleDeserializer = deserializer.deserializeArray(name);
        assert(tupleDeserializer);
        deserializeTupleElements(*tupleDeserializer, node);
    }

    void deserialize(ArrayDeserializer &deserializer, std::tuple<Types...> &node) const
    {
        ArrayDeserializer* tupleDeserializer = deserializer.deserializeArray();
        assert(tupleDeserializer);
        deserializeTupleElements(*tupleDeserializer, node);
    }
private:
    template<int elementNumber = 0,
             typename std::enable_if<elementNumber == sizeof...(Types), bool>::type = true>
    void serializeTupleElements(ArraySerializer&, const std::tuple<Types...>&) const
    {
    }

    template<int elementNumber = 0,
             typename std::enable_if<elementNumber != sizeof...(Types), bool>::type = true>
    void serializeTupleElements(ArraySerializer& serializer,
                                const std::tuple<Types...> &node) const
    {
        // TODO: maybe std::decay<T> is too aggressive and remove_cv<remove_reference<T>>
        // would be enough
        ObjectAssembler<typename std::decay<decltype(std::get<elementNumber>(node))>::type>::accessor().serialize(
            serializer, std::get<elementNumber>(node));

        serializeTupleElements<elementNumber + 1>(serializer, node);
    }

    template<int elementNumber = 0,
             typename std::enable_if<elementNumber == sizeof...(Types), bool>::type = true>
    void deserializeTupleElements(ArrayDeserializer&, std::tuple<Types...>&) const
    {
    }

    template<int elementNumber = 0,
             typename std::enable_if<elementNumber != sizeof...(Types), bool>::type = true>
    void deserializeTupleElements(ArrayDeserializer& deserializer,
                                  std::tuple<Types...> &node) const
    {
        ObjectAssembler<typename std::decay<decltype(std::get<elementNumber>(node))>::type>::accessor().deserialize(
            deserializer, std::get<elementNumber>(node));
        deserializeTupleElements<elementNumber + 1>(deserializer, node);
    }
};

#endif //SOFTEQ_COMMON_SERIALIZATION_TUPLE_OBJECT_ASSEMBLER_H
