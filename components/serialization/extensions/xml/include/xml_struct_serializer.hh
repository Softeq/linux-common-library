#ifndef SOFTEQ_COMMON_SERIALIZATION_XML_STRUCT_SERIALIZER_H
#define SOFTEQ_COMMON_SERIALIZATION_XML_STRUCT_SERIALIZER_H

#include "xml_utils.hh"

#include <common/serialization/serializers.hh>

#include <libxml/tree.h>

namespace softeq
{
namespace common
{
namespace serialization
{
namespace xml
{
/*!
    \brief The class is responsible for serialization of a structure
*/
class CompositeXmlSerializer : public StructSerializer
{
public:
    CompositeXmlSerializer();

    /*!
        \brief Creates a serializer object out of a node
        It is intended for internal use (by serializeStruct method)
        \param node which will be treated as a root node
    */
    explicit CompositeXmlSerializer(xmlNode *node);

    /*!
        \brief Creates a struct serializer for item with a specified name
        \param name name of the value
        \return StructSerializer object pointer. The ownership of the pointer belongs to this serializer
    */
    StructSerializer *serializeStruct(const std::string &name) override;

    /*!
        \brief Creates an array serializer for item with a specified name
        \param name name of the value
        \return ArrayDeserializer object pointer. The ownership of the pointer belongs to this serializer
    */
    ArraySerializer *serializeArray(const std::string &name) override;

    /*!
        \brief Creates a string representation of the serialized object
        \return serialized object as a string
    */
    std::string dump() const override;

private:
    /*!
        \brief Adds a value
        \param name of of the value to add
        \param value value to add
    */
    void serializeValueImpl(const std::string &name, const std::string &value) override;
    void serializeValueImpl(const std::string &name, int64_t value) override;
    void serializeValueImpl(const std::string &name, uint64_t value) override;
    void serializeValueImpl(const std::string &name, double value) override;
    void serializeValueImpl(const std::string &name, bool value) override;

    XmlDocUP _doc;
    xmlNode *_root;
};

} // namespace xml
} // namespace serialization
} // namespace common
} // namespace softeq

#endif
