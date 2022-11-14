#ifndef SOFTEQ_COMMON_SERIALIZATION_OBJECT_ASSEMBLER_H
#define SOFTEQ_COMMON_SERIALIZATION_OBJECT_ASSEMBLER_H

#include <common/stdutils/any.hh>
#include <common/stdutils/optional.hh>
#include <common/stdutils/stdutils.hh>

#include <common/serialization/base_member.hh>
#include <common/serialization/serializers.hh>
#include <common/serialization/deserializers.hh>

#include <limits>
#include <list>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <functional>

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

    template <typename SerializationType, typename Type>
    Self &defineAs(const std::string &name, Type Base::*reference,
                   SerializationType (*convertToSerializationType)(const Type &value),
                   Type (*convertFromSerializationType)(const SerializationType &value))
    {
        typename BaseMember<Base>::Ptr member(new CustomTypeMember<Base, Type, SerializationType>(
            name, reference, convertToSerializationType, convertFromSerializationType));
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
        /*TODO: check / implement nested serialization
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

    /*!
      Serialize whole node when the type of node is Struct
      \param serializer Current node serializer
      \param node The node to serialize
     */
    void serialize(StructSerializer &serializer, const Base &node) const
    {
        //      serializeMembers(serializer, node);
        for (const typename BaseMember<Base>::Ptr &it : _members)
        {
            it->serialize(serializer, node);
        }
    }

    /*!
      Serialize whole node when the type of node is Array
      \param serializer Current node serializer
      \param node The node to serialize
     */
    void serialize(ArraySerializer &serializer, const Base &node) const
    {
        StructSerializer *arrayObjectSerializer = serializer.serializeStruct();
        assert(arrayObjectSerializer);
        serialize(*arrayObjectSerializer, node);
    }

    template <typename... ARGS>
    void serialize(StructSerializer &serializer, const Base &node, ARGS... membersToSerialize) const
    {
        std::tuple<ARGS...> membersToSerializeContainer = {membersToSerialize...};
        partialSerialize(serializer, node, membersToSerializeContainer);
    }

    template <typename... ARGS>
    void serialize(ArraySerializer &serializer, const Base &node, ARGS... membersToSerialize) const
    {
        StructSerializer *objectElementSerializer = serializer.serializeStruct();
        assert(objectElementSerializer);
        serialize(*objectElementSerializer, node, membersToSerialize...);
    }

    // private:
    /*!
      Serialize element of the node. Used to go deeper.
      \param serializer Current node serializer
      \param node The node to serialize
     */
    void serializeElement(StructSerializer &serializer, const std::string &name, const Base &node) const
    {
        StructSerializer *nestedSerializer = serializer.serializeStruct(name);
        assert(nestedSerializer);
        serialize(*nestedSerializer, node);
    }

    // public:
    /*
        void serializeMembers(StructSerializer &serializer, const Base &node) const
        {
            for (const typename BaseMember<Base>::Ptr &it : _members)
            {
                it->serialize(serializer, node);
            }
        }
    */
    void deserialize(StructDeserializer &deserializer, Base &node) const
    {
        for (const typename BaseMember<Base>::Ptr &it : _members)
        {
            it->deserialize(deserializer, node);
        }
        //        deserializeMembers(deserializer, node);
    }

    void deserialize(ArrayDeserializer &deserializer, Base &node) const
    {
        StructDeserializer *arrayObjectDeserializer = deserializer.deserializeStruct();
        assert(arrayObjectDeserializer);
        deserialize(*arrayObjectDeserializer, node);
    }

    template <typename... ARGS>
    void deserialize(StructDeserializer &serializer, Base &node, ARGS... membersToDeserialize) const
    {
        std::tuple<ARGS...> membersToDeserializeContainer = {membersToDeserialize...};
        partialDeserialize(serializer, node, membersToDeserializeContainer);
    }

    template <typename... ARGS>
    void deserialize(ArrayDeserializer &deserializer, Base &node, ARGS... membersToDeserialize) const
    {
        StructDeserializer *structElementDeserializer = deserializer.deserializeStruct();
        assert(structElementDeserializer);
        deserialize(*structElementDeserializer, node, membersToDeserialize...);
    }

    void deserializeElement(StructDeserializer &deserializer, const std::string &name, Base &node) const
    {
        StructDeserializer *nextLevelDeserializer = deserializer.deserializeStruct(name);
        assert(nextLevelDeserializer);
        deserialize(*nextLevelDeserializer, node);
    }
    /*
        void deserializeMembers(StructDeserializer &deserializer, Base &node) const
        {
            for (const typename BaseMember<Base>::Ptr &it : _members)
            {
                it->deserialize(deserializer, node);
            }
        }
    */
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
    /*!
      used for extend
    */
    template <typename StructType, typename ExtendedStruct>
    class Ancestor : public BaseMember<StructType>
    {
    public:
        Ancestor(const std::string &name)
            : BaseMember<StructType>(name)
        {
        }

        void serialize(StructSerializer &serializer, const StructType &node) const override
        {
            ObjectAssembler<ExtendedStruct>::accessor().serialize(serializer, node);
        }

        void deserialize(StructDeserializer &deserializer, Base &node) const override
        {
            ObjectAssembler<ExtendedStruct>::accessor().deserialize(deserializer, node);
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
        friend class ObjectAssembler<Type, typename std::string>;

    public:
        TypedMember(const std::string &name, Type StructBase::*member)
            : BaseMember<StructBase>(name)
            , _member(member)
        {
        }

        void serialize(StructSerializer &serializer, const StructBase &node) const override
        {
            ObjectAssembler<Type>::accessor().serializeElement(serializer, this->name(), (node.*_member));
        }

        void deserialize(StructDeserializer &deserializer, Base &node) const override
        {
            try
            {
                ObjectAssembler<Type>::accessor().deserializeElement(deserializer, this->name(), (node.*_member));
            }
            catch (const ParseException &ex)
            {
                throw ParseException(this->name(), stdutils::string_format("Nested Exception :%s", ex.what()));
            }
            catch (const std::exception &ex)
            {
                throw ParseException(this->name(), stdutils::string_format("SDT Exception :%s", ex.what()));
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

    protected:
        Type StructBase::*_member;
    };

    template <typename StructBase, typename Type, typename CustomType>
    class CustomTypeMember : public TypedMember<StructBase, Type>
    {
    public:
        CustomTypeMember(const std::string &name, Type StructBase::*member,
                         std::function<CustomType(const Type &)> convertToCustomType,
                         std::function<Type(const CustomType &)> convertFromCustomType)
            : TypedMember<StructBase, Type>(name, member)
            , _convertToCustomType(std::move(convertToCustomType))
            , _convertFromCustomType(std::move(convertFromCustomType))
        {
        }

        void serialize(StructSerializer &serializer, const StructBase &node) const override
        {
            ObjectAssembler<CustomType>::accessor().serializeElement(
                serializer, this->name(), _convertToCustomType(node.*(TypedMember<StructBase, Type>::_member)));
        }

        void deserialize(StructDeserializer &deserializer, Base &node) const override
        {
            CustomType deserializedValue;
            ObjectAssembler<CustomType>::accessor().deserializeElement(deserializer, this->name(), deserializedValue);
            node.*(TypedMember<StructBase, Type>::_member) = _convertFromCustomType(deserializedValue);
        }

    private:
        std::function<CustomType(const Type &)> _convertToCustomType;
        std::function<Type(const CustomType &)> _convertFromCustomType;
    };

    // helper function to get member type. No implementation
    template <class Type>
    Type memberType(Type Base::*reference) const;

    template <typename SerializerType, std::size_t INDEX = 0, typename... ARGS,
              typename std::enable_if<INDEX == sizeof...(ARGS), int>::type = 0>
    void partialSerialize(const SerializerType & /*serializer*/, const Base & /*node*/,
                          const std::tuple<ARGS...> & /*membersToSerialize*/) const
    {
        // A special case.
        // This methos is empty since it signals that
        // all members in the tuple are handled and we
        // need to stop the recursion.
    }

    template <std::size_t INDEX = 0, typename... ARGS>
    using EnableIf = typename std::enable_if < INDEX<sizeof...(ARGS), int>::type;

    template <typename SerializerType, std::size_t INDEX = 0, typename... ARGS, EnableIf<INDEX, ARGS...> = 0>
    void partialSerialize(SerializerType &serializer, const Base &node, std::tuple<ARGS...> &membersToSerialize) const
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

        partialSerialize<SerializerType, INDEX + 1, ARGS...>(serializer, node, membersToSerialize);
    }

    template <typename SerializerType, std::size_t INDEX = 0, typename... ARGS,
              typename std::enable_if<INDEX == sizeof...(ARGS), int>::type = 0>
    void partialDeserialize(SerializerType &, Base &, std::tuple<ARGS...> &) const
    {
    }

    template <typename SerializerType, std::size_t INDEX = 0, typename... ARGS, EnableIf<INDEX, ARGS...> = 0>
    void partialDeserialize(SerializerType &serializer, Base &node, std::tuple<ARGS...> &membersToDeserialize) const
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

        partialDeserialize<SerializerType, INDEX + 1, ARGS...>(serializer, node, membersToDeserialize);
    }

    using MembersList = std::list<typename BaseMember<Base>::Ptr>;
    MembersList _members;
};

#include "details/assembler_specializations/arithmetic.hh"
#include "details/assembler_specializations/string.hh"
#include "details/assembler_specializations/list_or_vector.hh"
#include "details/assembler_specializations/optional.hh"
#include "details/assembler_specializations/maps.hh"
#include "details/assembler_specializations/enum.hh"
#include "details/assembler_specializations/tuple.hh"

} // namespace serialization
} // namespace common
} // namespace softeq

#endif // SOFTEQ_COMMON_SERIALIZATION_OBJECT_ASSEMBLER_H
