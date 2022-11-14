#ifndef SOFTEQ_COMMON_SERIALIZATION_XML_ARRAY_H
#define SOFTEQ_COMMON_SERIALIZATION_XML_ARRAY_H

#include <common/serialization/serializers.hh>

#include "xml_utils.hh"

namespace softeq
{
namespace common
{
namespace serialization
{
namespace xml
{
/*!
    \brief The class is responsible for array serialization.
*/
class XmlArraySerializer : public ArraySerializer
{
public:
    XmlArraySerializer();

    /*!
        \brief Constructs an object out of a XML node
        \param node XML node
    */
    explicit XmlArraySerializer(xmlNode *node);

    ~XmlArraySerializer() override = default;

    /*!
        \brief Creates a string representation of the serialized object
        \return serialized object as a string
    */
    std::string dump() const override;

protected:
    /*!
        \brief Adds a value to the array
        \param value value to add
    */
    void serializeValueImpl(int64_t value) override;
    void serializeValueImpl(uint64_t value) override;
    void serializeValueImpl(double value) override;
    void serializeValueImpl(bool value) override;
    void serializeValueImpl(const std::string &value) override;

    /*!
        \brief Adds an empty (null) value to the array
    */
    void serializeEmpty() override;

    /*!
        \brief Creates an array serializer for current item
        \return ArrayDeserializer object pointer. The ownership of the pointer belongs to this serializer
    */
    ArraySerializer *serializeArray() override;

    /*!
        \brief Creates a struct serializer for current item
        \return StructSerializer object pointer. The ownership of the pointer belongs to this serializer
    */
    StructSerializer *serializeStruct() override;

    /*!
        \brief Returns a root node
        \return root node
    */
    xmlNode *root() const;

private:
    XmlDocUP _doc;
    xmlNode *_root;

    // arrays are homogenous, but for some reason we specify the type to each element
    // it brings the need of remembering the type because sometimes we need to insert an empty
    // value which type cannot be inferred in another way
    std::string _type;
};

/*!
    \brief The class is responsible for array serialization when the root element is an array,
    e.g. <root><___containerEntry___ type="int">10</___containerEntry___></root>
*/
class RootXmlArraySerializer : public XmlArraySerializer
{
public:
    RootXmlArraySerializer() = default;
    ~RootXmlArraySerializer() override = default;

private:
    ArraySerializer *serializeArray() override;
};

} // namespace xml
} // namespace serialization
} // namespace common
} // namespace softeq

#endif // SOFTEQ_COMMON_SERIALIZATION_XML_ARRAY_H
