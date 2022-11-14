#ifndef SOFTEQ_COMMON_SERIALIZATION_XML_STRUCT_DESERIALIZER_H
#define SOFTEQ_COMMON_SERIALIZATION_XML_STRUCT_DESERIALIZER_H

#include "xml_utils.hh"

#include <common/serialization/deserializers.hh>

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
    \brief The class is responsible for deserialization of a structure.
*/
class CompositeXmlDeserializer : public StructDeserializer
{
public:
    CompositeXmlDeserializer() = default;

    /*!
        \brief Creates a deserializer object using XML document and XML node
        \param doc XML document
        \param node root node
    */
    explicit CompositeXmlDeserializer(XmlDocSP doc, const xmlNode *node);

    ~CompositeXmlDeserializer() override = default;

    /*!
        \brief Sets an XML text input to parse
        \param textInput XML input
    */
    void setRawInput(const std::string &textInput) override;

    /*!
        \brief Checks if a value with a specified name exists in the XML document
        \param name name of the value
        \return true if exists
    */
    bool valueExists(const std::string &name) const override;

    /*!
        \brief Returns the vector of all available value names in root XML node
        \return a vector of names
    */
    std::vector<std::string> availableNames() const override;

    /*!
        \brief Return a value of a given name
        \param name name of the value
        \return Any object (can be null)
    */
    softeq::common::stdutils::Any value(const std::string &name) override;

    /*!
        \brief Creates a struct serializer for an element of a given name
        \param name name of the value
        \return StructDeserializer object pointer. The ownership of the pointer belongs to this serializer
    */
    StructDeserializer *deserializeStruct(const std::string &name) override;

    /*!
        \brief Creates aa array serializer for  an element of a given name
        \param name name of the value
        \return ArrayDeserializer object pointer. The ownership of the pointer belongs to this serializer
    */
    ArrayDeserializer *deserializeArray(const std::string &name) override;

    /*!
        \brief Initializes XML multithreading (calls xmlInitParser)
    */
    static void initMultiThreading();

    /*!
        \brief Cleanups XML multithreading (calls xmlCleanupParser)
    */
    static void cleanup();

private:
    XmlDocSP _doc;
    XmlNodeMap _nodes;
};

} // namespace xml
} // namespace serialization
} // namespace common
} // namespace softeq

#endif
