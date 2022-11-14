#ifndef SOFTEQ_COMMON_SERIALIZATION_XML_ARRAY_DESERIALIZER_H
#define SOFTEQ_COMMON_SERIALIZATION_XML_ARRAY_DESERIALIZER_H

#include <common/serialization/deserializers.hh>

#include "xml_utils.hh"

#include <libxml/parser.h>

#include <map>

namespace softeq
{
namespace common
{
namespace serialization
{
namespace xml
{
/*!
    \brief The class is responsible for array deserialization.
*/
class XmlArrayDeserializer : public ArrayDeserializer
{
public:
    XmlArrayDeserializer() = default;

    /*!
        \brief Create a new array serializer out of existing document and root node.
        \param doc shared pointer to document
        \param node the node that should be considered root
    */
    explicit XmlArrayDeserializer(XmlDocSP doc, xmlNode *node);

    ~XmlArrayDeserializer() override = default;

    /*!
        \brief sets the text input to parse
        \param textInput text input containing xml
    */
    void setRawInput(const std::string &textInput) override;

    /*!
        \brief return the current value and advances to the next one
        \return Any object containing the value or nothing
    */
    softeq::common::stdutils::Any value() override;

    /*!
        \brief Check if there are any more items available in the array.
        \return true if there are more items to fetch
    */
    bool isComplete() const override;

    /*!
        \brief Checks if there is a next value.
        \param true if there is a values available and its not null
    */
    bool nextValueExists() const override;

    /*!
        \brief Creates a struct serializer for current item
        \return StructDeserializer object pointer. The ownership of the pointer belongs to this serializer
    */
    StructDeserializer *deserializeStruct() override;

    /*!
        \brief Creates aa array serializer for current item
        \return ArrayDeserializer object pointer. The ownership of the pointer belongs to this serializer
    */
    ArrayDeserializer *deserializeArray() override;

    /*!
        \brief Gets current index in the array
    */
    std::size_t index() const override;

protected:
    XmlDocSP doc() const;
    xmlNode *element() const;

private:
    XmlDocSP _doc;
    xmlNode *_node = nullptr;
    std::size_t _index = 0;
};

/*!
    \brief The class is responsible for array deserialization when the root element is an array,
    e.g. <root><___containerEntry___ type="int">10</___containerEntry___></root>
*/
class RootXmlArrayDeserializer : public XmlArrayDeserializer
{
public:
    RootXmlArrayDeserializer() = default;
    ~RootXmlArrayDeserializer() override = default;

private:
    /*!
        \brief Creates aa array serializer for current item
        \return ArrayDeserializer object pointer. The ownership of the pointer belongs to this serializer
    */
    ArrayDeserializer *deserializeArray() override;
};

} // namespace xml
} // namespace serialization
} // namespace common
} // namespace softeq

#endif
