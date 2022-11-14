#ifndef SOFTEQ_COMMON_SERIALIZATION_XML_UTILS_H
#define SOFTEQ_COMMON_SERIALIZATION_XML_UTILS_H

#include <common/serialization/deserializers.hh>
#include <common/stdutils/any.hh>

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
namespace literals
{
constexpr static const char *typeProperty = "type";
constexpr static const char *signedIntegerType = "int";
constexpr static const char *unsignedIntegerType = "uint";
constexpr static const char *floatingType = "float";
constexpr static const char *stringType = "string";
constexpr static const char *booleanType = "bool";
constexpr static const char *booleanTypeTrue = "true";
constexpr static const char *booleanTypeFalse = "false";
constexpr static const char *containerEntry = "___containerEntry___";

constexpr const char *rootNodeName = "root";

} // namespace literals

struct XmlDocDeleter
{
    void operator()(xmlDoc *doc) const;
};
struct XmlCharDeleter
{
    void operator()(xmlChar *ptr) const;
};

using XmlNodeMap = std::map<std::string, xmlNodePtr>;
using XmlDocSP = std::shared_ptr<xmlDoc>;
using XmlDocUP = std::unique_ptr<xmlDoc, XmlDocDeleter>;
using XmlCharUP = std::unique_ptr<xmlChar, XmlCharDeleter>;

/*!
    \brief Converts const xmlChar* to a std::string
*/
std::string xmlCharToString(const xmlChar *chars);

/*!
    \brief Converts a const char* to const xmlChar*
*/
const xmlChar *stringToXmlChar(const char *str);

/*!
    \brief Creates a node map out of a root node
    \param root node
    \return XML node map
*/
XmlNodeMap discoverNodes(const xmlNode *root);

/*!
    \brief Parses a node and returns it's value
    \param doc XML document
    \param node node to parse
    \return Any object (can be null)
*/
stdutils::Any nodeValue(xmlDoc *doc, xmlNode *node);

/*!
    \brief Checks if a node does not contain a value
    \return true if the node is empty
*/
bool isEmptyNode(xmlNode *node);

/*!
    \brief Creates a string from a XML document
    \param doc XML document
    \result XML string
*/
std::string dumpXml(xmlDoc *doc);

} // namespace xml
} // namespace serialization
} // namespace common
} // namespace softeq
#endif
