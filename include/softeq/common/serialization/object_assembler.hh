#ifndef SOFTEQ_COMMON_SERIALIZATION_OBJECT_ASSEMBLER_H
#define SOFTEQ_COMMON_SERIALIZATION_OBJECT_ASSEMBLER_H

#include <cassert>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <functional>

#include "softeq/common/any.hh"
#include "softeq/common/optional.hh"
#include <softeq/common/scope_guard.hh>
#include "softeq/common/stdutils.hh"
#include "softeq/common/serialization/base_member.hh"
#include "softeq/common/serialization/serializer.hh"

namespace softeq
{
namespace common
{
namespace serialization
{
template <typename T, typename Enable = void>
class ObjectAssembler;

template <typename T>
ObjectAssembler<T> Assembler();

template <typename Base, typename Enable>
class ObjectAssembler final
{
public:
    using Self = ObjectAssembler<Base>;

    inline static Self accessor()
    {
        return Assembler<Base>();
    }

    template <class Type>
    Self &define(const std::string &name, Type Base::*reference)
    {
        for (const typename BaseMember<Base>::Ptr &it : _members)
        {
            if (it->name() == name)
            {
                throw std::logic_error("redeclaration of memeber with name '" + name + "'");
            }

            const TypedMember<Base, Type> *member = dynamic_cast<const TypedMember<Base, Type> *>(it.get());
            if ((member != nullptr) && (member->isPtrMemberEqual(reference)))
            {
                throw std::logic_error("redeclaration of pointer memeber");
            }
        }

        typename BaseMember<Base>::Ptr member(new TypedMember<Base, Type>(name, reference));
        _members.push_back(member);
        return *this;
    }

    void forEach(std::function<void(const typename BaseMember<Base>::Ptr &)> &func)
    {
        for (const typename BaseMember<Base>::Ptr &it : _members)
        {
            func(it);
        }
    }

    template <typename ExtendedStruct>
    Self &extend(const std::string &name)
    {
        typename BaseMember<Base>::Ptr ptr(new Ancestor<Base, ExtendedStruct>(name));
        _members.push_back(ptr);
        /*
        ObjectAssembler<ExtendedStruct> ref(Assembler<ExtendedStruct>());
        std::function<void(const typename BaseMember<ExtendedStruct>::Ptr &it)> fn =
            [this](const typename BaseMember<ExtendedStruct>::Ptr &it) {
                const TypedMember<Base, Type> *member = dynamic_cast<const TypedMember<Base, Type> *>(it.get());
                _members.push_back(member);
                return;
            };

        ref.forEach(fn);*/
        return *this;
    }

    void serialize(Serializer &serializer, const Base &node) const
    {
        serializer.beginObjectSerialization(Serializer::CurrentObjectType::STRUCT);
        serializeMembers(serializer, node);
        serializer.endObjectSerialization();
    }

    template <typename... ARGS>
    void serialize(Serializer &serializer, const Base &node, ARGS... membersToSerialize) const
    {
        serializer.beginObjectSerialization(Serializer::CurrentObjectType::STRUCT);
        std::tuple<ARGS...> membersToSerializeContainer = {membersToSerialize...};
        partialSerialize(serializer, node, membersToSerializeContainer);
        serializer.endObjectSerialization();
    }

    void serializeMembers(Serializer &serializer, const Base &node) const
    {
        for (const typename BaseMember<Base>::Ptr &it : _members)
        {
            it->serialize(serializer, node);
        }
    }

    void deserialize(Serializer &serializer, Base &node)
    {
        serializer.beginObjectDeserialization(Serializer::CurrentObjectType::STRUCT);

        scope_guard guard([&serializer] { serializer.endObjectDeserialization(); });

        deserializeMembers(serializer, node);
    }

    template <typename... ARGS>
    void deserialize(Serializer &serializer, Base &node, ARGS... membersToDeserialize) const
    {
        serializer.beginObjectDeserialization(Serializer::CurrentObjectType::STRUCT);
        std::tuple<ARGS...> membersToDeserializeContainer = {membersToDeserialize...};

        scope_guard guard([&serializer] { serializer.endObjectDeserialization(); });

        partialDeserialize(serializer, node, membersToDeserializeContainer);
    }

    void deserializeMembers(Serializer &serializer, Base &node)
    {
        for (typename BaseMember<Base>::Ptr &it : _members)
        {
            it->deserialize(serializer, node);
        }
    }

    std::string graph() const
    {
        return "digraph struct {rankdir=LR; bgcolor=\"#ffffff00\"; "
               "node [fontname=\"sans\", fontsize=\"10\", fillcolor=\"#FFFF0000\", style=\"filled\", "
               "margin=\"0.1\"];" +
               graph("root") + "}";
    }

    std::string graph(const std::string &assignedNodeName) const
    {
        std::string result;
        result.reserve(4096);

        result +=
            assignedNodeName +
            " [shape=plaintext, style=\"\", "
            "label=<<TABLE BORDER=\"0\" BGCOLOR=\"lightgray\" CELLPADDING=\"3\" CELLBORDER=\"1\" CELLSPACING=\"0\">"
            "<TR><TD PORT=\"name\"><B>STRUCT</B></TD></TR>";
        size_t index = 0;
        for (const typename BaseMember<Base>::Ptr &it : _members)
        {
            result += "<TR><TD PORT=\"member" + std::to_string(index) + "\">";
            result += it->name();
            result += "</TD></TR>";
            index++;
        }
        result += "</TABLE>>];";

        index = 0;
        for (const typename BaseMember<Base>::Ptr &it : _members)
        {
            std::string childNodeName = assignedNodeName + "_" + it->name();
            std::string lineLength;
            result += assignedNodeName + ":member" + std::to_string(index) + " -> " + childNodeName + ":name;";
            result += it->graph(childNodeName);
            index++;
        }

        return result;
    }

private:
    template <typename StructType, typename ExtendedStruct>
    class Ancestor : public BaseMember<StructType>
    {
    public:
        Ancestor(const std::string &name)
            : BaseMember<StructType>(name)
        {
        }

        void serialize(Serializer &serializer, const StructType &node) const override
        {
            try
            {
                ObjectAssembler<ExtendedStruct>::accessor().serializeMembers(serializer, node);
            }
            catch (const std::exception &ex)
            {
                throw std::logic_error(
                    stdutils::string_format("Failed to serialize '%s' %s", this->name().c_str(), ex.what()));
            }
        }

        void deserialize(Serializer &serializer, StructType &node) override
        {
            try
            {
                ObjectAssembler<ExtendedStruct>::accessor().deserializeMembers(serializer, node);
            }
            catch (const std::exception &ex)
            {
                throw ParseException(
                    stdutils::string_format("Failed to deserialize '%s' %s", this->name().c_str(), ex.what()));
            }
        }

        std::string graph(const std::string &assignedNodeName) const override
        {
            return ObjectAssembler<ExtendedStruct>::accessor().graph(assignedNodeName);
        }

        bool operator==(const BaseMember<StructType> &member) const override
        {
            bool result;
            try
            {
                const Ancestor<StructType, ExtendedStruct> &ancestor =
                    dynamic_cast<const Ancestor<StructType, ExtendedStruct> &>(member);
                (void)ancestor;
                result = true;
            }
            catch (const std::bad_cast &e)
            {
                result = false;
            }

            return result;
        }

        const std::type_info &type() const override
        {
            return typeid(ExtendedStruct);
        }

    private:
    };

    template <typename StructBase, typename Type>
    class TypedMember : public BaseMember<StructBase>
    {
    public:
        TypedMember(const std::string &name, Type StructBase::*member)
            : BaseMember<StructBase>(name)
            , _member(member)
        {
        }

        void serialize(Serializer &serializer, const StructBase &node) const override
        {
            try
            {
                serializer.name(this->name());
                ObjectAssembler<Type>::accessor().serialize(serializer, (node.*_member));
            }
            catch (const std::exception &ex)
            {
                throw std::logic_error(
                    stdutils::string_format("Failed to serialize '%s' %s", this->name().c_str(), ex.what()));
            }
        }

        void deserialize(Serializer &serializer, Base &node) override
        {
            try
            {
                serializer.name(this->name());
                ObjectAssembler<Type>::accessor().deserialize(serializer, (node.*_member));
            }
            catch (const std::exception &ex)
            {
                throw ParseException(
                    stdutils::string_format("Failed to deserialize '%s' %s", this->name().c_str(), ex.what()));
            }
        }

        std::string graph(const std::string &assignedNodeName) const override
        {
            return ObjectAssembler<Type>::accessor().graph(assignedNodeName);
        }

        bool operator==(const BaseMember<Base> &member) const override
        {
            bool result;
            try
            {
                const TypedMember<Base, Type> &typedMember = dynamic_cast<const TypedMember<Base, Type> &>(member);
                result = (typedMember._member == this->_member) && (typedMember.name() == this->name());
            }
            catch (const std::bad_cast &e)
            {
                result = false;
            }

            return result;
        }

        const std::type_info &type() const override
        {
            return typeid(Type);
        }

        bool isPtrMemberEqual(Type StructBase::*member) const
        {
            return (member == _member);
        }

    private:
        Type StructBase::*_member;
    };

    // helper function to get member type. No implementation
    template <class Type>
    Type memberType(Type Base::*reference) const;

    template <std::size_t INDEX = 0, typename... ARGS, typename std::enable_if<INDEX == sizeof...(ARGS), int>::type = 0>
    void partialSerialize(const Serializer & /*serializer*/, const Base & /*node*/,
                          const std::tuple<ARGS...> & /*membersToSerialize*/) const
    {
        // A special case.
        // This methos is empty since it signals that
        // all members in the tuple are handled and we
        // need to stop the recursion.
    }

    template <std::size_t INDEX = 0, typename... ARGS>
    using EnableIf = typename std::enable_if < INDEX<sizeof...(ARGS), int>::type;

    template <std::size_t INDEX = 0, typename... ARGS, EnableIf<INDEX, ARGS...> = 0>
    void partialSerialize(Serializer &serializer, const Base &node, std::tuple<ARGS...> &membersToSerialize) const
    {
        bool found = false;
        for (const typename BaseMember<Base>::Ptr &it : _members)
        {
            using MemberType = decltype(memberType(std::get<INDEX>(membersToSerialize)));
            TypedMember<Base, MemberType> stub(it->name(), std::get<INDEX>(membersToSerialize));
            if (stub == *it)
            {
                it->serialize(serializer, node);
                found = true;
                break;
            }
        }

        if (found == false)
        {
            throw std::logic_error(std::string("pointer to member ") +
                                   typeid(std::get<INDEX>(membersToSerialize)).name() + " is not found");
        }

        partialSerialize<INDEX + 1, ARGS...>(serializer, node, membersToSerialize);
    }

    template <std::size_t INDEX = 0, typename... ARGS, typename std::enable_if<INDEX == sizeof...(ARGS), int>::type = 0>
    void partialDeserialize(Serializer &serializer, Base &node, std::tuple<ARGS...> &membersToDeserialize) const
    {
    }

    template <std::size_t INDEX = 0, typename... ARGS, EnableIf<INDEX, ARGS...> = 0>
    void partialDeserialize(Serializer &serializer, Base &node, std::tuple<ARGS...> &membersToDeserialize) const
    {
        bool found = false;
        for (const typename BaseMember<Base>::Ptr &it : _members)
        {
            using MemberType = decltype(memberType(std::get<INDEX>(membersToDeserialize)));
            TypedMember<Base, MemberType> stub(it->name(), std::get<INDEX>(membersToDeserialize));
            if (stub == *it)
            {
                it->deserialize(serializer, node);
                found = true;
                break;
            }
        }

        if (found == false)
        {
            throw std::logic_error(std::string("pointer to member ") +
                                   typeid(std::get<INDEX>(membersToDeserialize)).name() + " is not found");
        }

        partialDeserialize<INDEX + 1, ARGS...>(serializer, node, membersToDeserialize);
    }

    using MembersList = std::list<typename BaseMember<Base>::Ptr>;
    MembersList _members;
};

template <typename T>
class ObjectAssembler<T, typename std::enable_if<std::is_arithmetic<T>::value>::type> final
{
public:
    inline static ObjectAssembler<T> accessor()
    {
        return ObjectAssembler<T>();
    }

    void serialize(Serializer &serializer, const T &node) const
    {
        serializer.value(node);
    }

    void deserialize(Serializer &serializer, T &node)
    {
        deserialize<T>(serializer, node);
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
    void deserialize(Serializer &serializer, T2 &node)
    {
        Any any = serializer.value();
        assert(any.hasValue());
        node = any_cast<bool>(any);
    }

    template <typename T2,
              typename std::enable_if<!std::is_same<bool, T2>::value && std::is_integral<T2>::value, int>::type = 0>
    void deserialize(Serializer &serializer, T2 &node)
    {
        Any any = serializer.value();
        assert(any.hasValue());
        if (any.type() == typeid(int64_t))
        {
            int64_t value = any_cast<int64_t>(any);
            if (std::is_unsigned<T2>::value)
            {
                if (value < 0)
                {
                    throw ParseException("Out of range: value=" + std::to_string(value) +
                                         " is less than 0 for unsigned value");
                }
                else if (static_cast<uint64_t>(value) > std::numeric_limits<T2>::max())
                {
                    throw ParseException("Out of range: value=" + std::to_string(value) +
                                         " is bigger than max value (" +
                                         std::to_string(std::numeric_limits<T2>::max()) + ")");
                }
            }
            else
            {
                if (value < std::numeric_limits<T2>::min())
                {
                    throw ParseException("Out of range: value=" + std::to_string(value) + " is less than min value (" +
                                         std::to_string(std::numeric_limits<T2>::min()) + ")");
                }
                else if (value > std::numeric_limits<T2>::max())
                {
                    throw ParseException("Out of range: value=" + std::to_string(value) +
                                         " is bigger than max value (" +
                                         std::to_string(std::numeric_limits<T2>::max()) + ")");
                }
            }

            node = static_cast<T2>(value);
        }
        else if (any.type() == typeid(uint64_t))
        {
            uint64_t value = any_cast<uint64_t>(any);
            if (value > static_cast<uint64_t>(std::numeric_limits<T2>::max()))
            {
                throw ParseException("Out of range: value=" + std::to_string(value) + " is bigger than max value (" +
                                     std::to_string(std::numeric_limits<T2>::max()) + ")");
            }

            node = static_cast<T2>(value);
        }
        else
        {
            throw ParseException("Not integer value");
        }
    }

    template <typename T2, typename std::enable_if<std::is_floating_point<T2>::value, int>::type = 0>
    void deserialize(Serializer &serializer, T2 &node)
    {
        Any any = serializer.value();
        assert(any.hasValue());
        try
        {
            node = static_cast<T2>(any_cast<double>(any));
        }
        catch (const std::bad_cast &e)
        {
            try
            {
                node = static_cast<T2>(any_cast<uint64_t>(any));
            }
            catch (const std::bad_cast &e)
            {
                try
                {
                    node = static_cast<T2>(any_cast<int64_t>(any));
                }
                catch (const std::bad_cast &e)
                {
                    throw ParseException(
                        stdutils::string_format("Failed to deserialize a floating point value. %s", e.what()));
                }
            }
        }
    }
};

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

    void serialize(Serializer &serializer, const std::string &node) const
    {
        serializer.value(node);
    }

    void deserialize(Serializer &serializer, std::string &node)
    {
        Any any = serializer.value();
        assert(any.hasValue());
        node = any_cast<std::string>(any);
    }
};

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

    void serialize(Serializer &serializer, const T &node) const
    {
        serializer.beginObjectSerialization(Serializer::CurrentObjectType::ARRAY);
        for (auto &it : node)
        {
            ObjectAssembler<typename T::value_type>::accessor().serialize(serializer, it);
        }
        serializer.endObjectSerialization();
    }

    void deserialize(Serializer &serializer, T &node)
    {
        size_t size = serializer.beginObjectDeserialization(Serializer::CurrentObjectType::ARRAY);
        scope_guard guard([&serializer] { serializer.endObjectDeserialization(); });

        node.clear();
        for (size_t i = 0; i < size; i++)
        {
            node.push_back({});
            ObjectAssembler<typename T::value_type>::accessor().deserialize(serializer, node.back());
        }
    }
};

template <typename T>
class ObjectAssembler<Optional<T>> final
{
public:
    inline static ObjectAssembler<Optional<T>> accessor()
    {
        return ObjectAssembler<Optional<T>>();
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

    void serialize(Serializer &serializer, const Optional<T> &node) const
    {
        if (node.hasValue())
        {
            ObjectAssembler<T>::accessor().serialize(serializer, node.cValue());
        }
    }

    void deserialize(Serializer &serializer, Optional<T> &node)
    {
        try
        {
            T value;
            ObjectAssembler<T>::accessor().deserialize(serializer, value);
            node = Optional<T>(value);
        }
        catch (const NullNodeException &e)
        {
            node = Optional<T>();
        }
        catch (const ParseException &e)
        {
            node = Optional<T>();
        }
    }
};

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

    void serialize(Serializer &serializer, const T &node) const
    {
        serializer.beginObjectSerialization(Serializer::CurrentObjectType::ARRAY);
        for (auto &it : node)
        {
            serializer.beginObjectSerialization(Serializer::CurrentObjectType::STRUCT);
            serializer.name(cKeyName);
            ObjectAssembler<typename T::key_type>::accessor().serialize(serializer, it.first);
            serializer.name(cValueName);
            ObjectAssembler<typename T::mapped_type>::accessor().serialize(serializer, it.second);
            serializer.endObjectSerialization();
        }
        serializer.endObjectSerialization();
    }

    void deserialize(Serializer &serializer, T &node)
    {
        size_t size = serializer.beginObjectDeserialization(Serializer::CurrentObjectType::ARRAY);
        scope_guard guard([&serializer] { serializer.endObjectDeserialization(); });

        node.clear();
        for (size_t i = 0; i < size; i++)
        {
            typename T::key_type key;
            typename T::mapped_type value;

            serializer.beginObjectDeserialization(Serializer::CurrentObjectType::STRUCT);
            scope_guard loopGuard([&serializer] { serializer.endObjectDeserialization(); });

            serializer.name(cKeyName);
            ObjectAssembler<typename T::key_type>::accessor().deserialize(serializer, key);

            serializer.name(cValueName);
            ObjectAssembler<typename T::mapped_type>::accessor().deserialize(serializer, value);
            node[key] = std::move(value);
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

    void serialize(Serializer &serializer, const T &node) const
    {
        serializer.beginObjectSerialization(Serializer::CurrentObjectType::STRUCT);
        for (auto &it : node)
        {
            serializer.name(it.first);
            ObjectAssembler<typename T::mapped_type>::accessor().serialize(serializer, it.second);
        }
        serializer.endObjectSerialization();
    }

    void deserialize(Serializer &serializer, T &node)
    {
        serializer.beginObjectDeserialization(Serializer::CurrentObjectType::STRUCT);
        scope_guard guard([&serializer] { serializer.endObjectDeserialization(); });

        node.clear();
        const std::vector<std::string> keys = serializer.getNodeKeys();
        for (const std::string &key : keys)
        {
            serializer.name(key);

            typename T::mapped_type value;
            ObjectAssembler<typename T::mapped_type>::accessor().deserialize(serializer, value);
            node[key] = std::move(value);
        }
    }
};

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

    void serialize(Serializer &serializer, const T &node) const
    {
        typename ValueMap::const_iterator it = _map.begin();
        for (; it != _map.end(); it++)
        {
            if (it->second == node)
            {
                break;
            }
        }

        if (it == _map.end())
        {
            throw std::runtime_error(std::string("No mapped value ") + valueAsString(node) + " specified for type " +
                                     stdutils::demangle(typeid(T).name()));
        }

        serializer.value(it->first);
    }

    void deserialize(Serializer &serializer, T &node)
    {
        const Any any = serializer.value();
        assert(any.hasValue());
        const std::string value = any_cast<std::string>(any);
        typename ValueMap::const_iterator it = _map.find(value);
        if (it != _map.end())
        {
            node = it->second;
            return;
        }

        throw std::runtime_error("Unknown enumeration value");
    }

    std::string graph(const std::string &assignedNodeName) const
    {
        if (_map.empty())
        {
            throw std::runtime_error(std::string("The enum map (") + stdutils::demangle(typeid(T).name()) +
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
};

} // namespace serialization
} // namespace common
} // namespace softeq

#endif // SOFTEQ_COMMON_SERIALIZATION_OBJECT_ASSEMBLER_H
