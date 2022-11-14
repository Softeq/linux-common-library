#include "xml_utils.hh"

namespace softeq
{
namespace common
{
namespace serialization
{
namespace xml
{
XmlNodeMap discoverNodes(const xmlNode *root)
{
    XmlNodeMap nodes;
    for (xmlNode *node = root->xmlChildrenNode; node != nullptr; node = node->next)
    {
        if (node->type == XML_ELEMENT_NODE)
        {
            nodes[xmlCharToString(node->name)] = node;
        }
    }
    return nodes;
}

stdutils::Any parseValue(const std::string &content, const std::string &type)
{
    stdutils::Any any;
    try
    {
        if (type == literals::signedIntegerType)
        {
            any = std::stol(content);
        }
        else if (type == literals::unsignedIntegerType)
        {
            // if content starts with '-', handle as signed type to prevent wraparound by std::stoul
            if ((content.size() > 1) && (content[0] == '-'))
            {
                any = std::stol(content);
            }
            else
            {
                any = std::stoul(content);
            }
        }
        else if (type == literals::floatingType)
        {
            double value = std::stod(content);
            any = value;
        }
        else if (type == literals::booleanType)
        {
            if (content == literals::booleanTypeTrue)
            {
                any = true;
            }
            else if (content == literals::booleanTypeFalse)
            {
                any = false;
            }
        }
        else if (type == literals::stringType)
        {
            any = content;
        }
        else
        {
            throw ParseException("", "Not supported primitive type '" + type + "'");
        }
    }
    catch (std::invalid_argument &)
    {
        // do nothing - result will be null
    }
    catch (std::out_of_range &)
    {
        // do nothing - result will be null
    }
    catch (...)
    {
        throw; // re-throw the rest
    }
    return any;
}

stdutils::Any nodeValue(xmlDoc *doc, xmlNode *node)
{
    stdutils::Any any{};
    XmlCharUP typeProp(xmlGetProp(node, stringToXmlChar(literals::typeProperty)));
    if (typeProp) // if the node has a 'type' attribute, it is a primitive value
    {
        XmlCharUP contentUP(xmlNodeListGetString(doc, node->xmlChildrenNode, 1));

        if (contentUP)
        {
            std::string type = xmlCharToString(typeProp.get());
            std::string content = xmlCharToString(contentUP.get());

            any = parseValue(content, type);
        }
    }
    return any;
}

bool isEmptyNode(xmlNode *node)
{
    return node->xmlChildrenNode == nullptr;
}

std::string dumpXml(xmlDoc *doc)
{
    xmlChar *mem{};
    int size{};
    xmlDocDumpMemory(doc, &mem, &size);
    std::string dump(reinterpret_cast<const char *>(mem), size);
    xmlFree(mem);
    return dump;
}

void XmlDocDeleter::operator()(xmlDoc *doc) const
{
    xmlFreeDoc(doc);
}

void XmlCharDeleter::operator()(xmlChar *ptr) const
{
    xmlFree(ptr);
}

std::string xmlCharToString(const xmlChar *chars)
{
    return reinterpret_cast<const char *>(chars);
}

const xmlChar *stringToXmlChar(const char *str)
{
    return reinterpret_cast<const xmlChar *>(str);
}

} // namespace xml
} // namespace serialization
} // namespace common
} // namespace softeq
