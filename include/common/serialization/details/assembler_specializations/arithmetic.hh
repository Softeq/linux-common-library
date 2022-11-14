#ifndef SOFTEQ_COMMON_SERIALIZATION_ARITHMETIC_OBJECT_ASSEMBLER_H
#define SOFTEQ_COMMON_SERIALIZATION_ARITHMETIC_OBJECT_ASSEMBLER_H

template <typename T>
class ObjectAssembler<T, typename std::enable_if<std::is_arithmetic<T>::value>::type> final
{
public:
    inline static ObjectAssembler<T> accessor()
    {
        return ObjectAssembler<T>();
    }

    void serialize(ArraySerializer &serializer, const T &node) const
    {
        serializer.serializeValue(node);
    }

    void serializeElement(StructSerializer &serializer, const std::string &name, const T &value) const
    {
        serializer.serializeValue(name, value);
    }

    void deserializeElement(StructDeserializer &deserializer, const std::string &name, T &node) const
    {
        softeq::common::stdutils::Any value = std::move(deserializer.value(name));
        if (!value.hasValue())
        {
            throw std::logic_error("Expected node, but not provided");
        }
        extractCorrectValueOfType<T>(value, node);
    }

    void deserialize(ArrayDeserializer &deserializer, T &node) const
    {
        extractCorrectValueOfType<T>(deserializer.value(), node);
    }

    std::string graph(const std::string &assignedNodeName) const
    {
        return createSampleImpl<T>(assignedNodeName);
    }

private:
    template <typename T2>
    typename std::enable_if<std::is_same<bool, T2>::value, std::string>::type
    createSampleImpl(const std::string &assignedNodeName) const
    {
        return assignedNodeName + " [shape=note,label=<<I>boolean<BR/>true or false</I>>, height=0];";
    }

    template <typename T2>
    typename std::enable_if<
        !std::is_same<bool, T2>::value && std::is_integral<T2>::value && std::is_unsigned<T2>::value, std::string>::type
    createSampleImpl(const std::string &assignedNodeName) const
    {
        return assignedNodeName + " [shape=note,label=<<I>unsigned<BR/>arithmetic</I>>, height=0];";
    }

    template <typename T2>
    typename std::enable_if<!std::is_same<bool, T2>::value && std::is_integral<T2>::value && std::is_signed<T2>::value,
                            std::string>::type
    createSampleImpl(const std::string &assignedNodeName) const
    {
        return assignedNodeName + " [shape=note,label=<<I>signed<BR/>arithmetic</I>>, height=0];";
    }

    template <typename T2>
    typename std::enable_if<std::is_floating_point<T2>::value, std::string>::type
    createSampleImpl(const std::string &assignedNodeName) const
    {
        return assignedNodeName + " [shape=note,label=<<I>floating-point<BR/>arithmetic</I>>, height=0];";
    }

    // Serializer uses as wide types as possible to parse values.
    // Use narrow conversion to get requested type in deserialize() functions.
    template <typename T2, typename std::enable_if<std::is_same<bool, T2>::value, int>::type = 0>
    void extractCorrectValueOfType(const softeq::common::stdutils::Any &any, T2 &node) const
    {
        node = softeq::common::stdutils::any_cast<bool>(any);
    }

    template <typename T2,
              typename std::enable_if<!std::is_same<bool, T2>::value && std::is_integral<T2>::value, int>::type = 0>
    void extractCorrectValueOfType(const softeq::common::stdutils::Any &any, T2 &node) const
    {
        if (any.type() == typeid(int64_t))
        {
            int64_t value = softeq::common::stdutils::any_cast<int64_t>(any);
            if (std::is_unsigned<T2>::value)
            {
                if (value < 0)
                {
                    throw std::out_of_range("value=" + std::to_string(value) + " is less than 0 for unsigned value");
                }
                else if (static_cast<uint64_t>(value) > std::numeric_limits<T2>::max())
                {
                    throw std::out_of_range("value=" + std::to_string(value) + " is bigger than max value (" +
                                            std::to_string(std::numeric_limits<T2>::max()) + ")");
                }
            }
            else
            {
                if (value < std::numeric_limits<T2>::min())
                {
                    throw std::out_of_range("value=" + std::to_string(value) + " is less than min value (" +
                                            std::to_string(std::numeric_limits<T2>::min()) + ")");
                }
                else if (value > std::numeric_limits<T2>::max())
                {
                    throw std::out_of_range("value=" + std::to_string(value) + " is bigger than max value (" +
                                            std::to_string(std::numeric_limits<T2>::max()) + ")");
                }
            }

            node = static_cast<T2>(value);
        }
        else if (any.type() == typeid(uint64_t))
        {
            uint64_t value = softeq::common::stdutils::any_cast<uint64_t>(any);
            if (value > static_cast<uint64_t>(std::numeric_limits<T2>::max()))
            {
                throw std::out_of_range("value=" + std::to_string(value) + " is bigger than max value (" +
                                        std::to_string(std::numeric_limits<T2>::max()) + ")");
            }

            node = static_cast<T2>(value);
        }
        else
        {
            throw std::logic_error("Not integral value");
        }
    }

    template <typename T2, typename std::enable_if<std::is_floating_point<T2>::value, int>::type = 0>
    void extractCorrectValueOfType(const softeq::common::stdutils::Any &any, T2 &node) const
    {
        try
        {
            node = static_cast<T2>(softeq::common::stdutils::any_cast<double>(any));
        }
        catch (const std::bad_cast &e)
        {
            try
            {
                node = static_cast<T2>(softeq::common::stdutils::any_cast<uint64_t>(any));
            }
            catch (const std::bad_cast &e)
            {
                // can throw again in case of bad_cast
                node = static_cast<T2>(softeq::common::stdutils::any_cast<int64_t>(any));
            }
        }
    }
};

#endif // SOFTEQ_COMMON_SERIALIZATION_ARITHMETIC_OBJECT_ASSEMBLER_H
