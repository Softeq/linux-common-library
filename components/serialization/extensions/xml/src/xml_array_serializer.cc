#include "xml_array_serializer.hh"
#include "xml_struct_serializer.hh"

using namespace softeq::common;
using namespace softeq::common::serialization::xml;

XmlArraySerializer::XmlArraySerializer(xmlNode *node)
    : _root(node)
{
}

XmlArraySerializer::XmlArraySerializer()
{
    _doc.reset(xmlNewDoc(nullptr));
    _root = xmlNewDocNode(_doc.get(), nullptr, stringToXmlChar(literals::rootNodeName), nullptr);
    xmlDocSetRootElement(_doc.get(), _root);
}

namespace
{
void addNode(xmlNode *node, const std::string &value, const std::string &type)
{
    xmlNode *child =
        xmlNewChild(node, nullptr, stringToXmlChar(literals::containerEntry), stringToXmlChar(value.c_str()));
    xmlNewProp(child, stringToXmlChar(literals::typeProperty), stringToXmlChar(type.c_str()));
}
xmlNode *addNode(xmlNode *node)
{
    return xmlNewChild(node, nullptr, stringToXmlChar(literals::containerEntry), nullptr);
}

} // namespace

void XmlArraySerializer::serializeValueImpl(int64_t value)
{
    _type = literals::signedIntegerType;
    addNode(_root, std::to_string(value), _type);
}

void XmlArraySerializer::serializeValueImpl(uint64_t value)
{
    _type = literals::unsignedIntegerType;
    addNode(_root, std::to_string(value), _type);
}

void XmlArraySerializer::serializeValueImpl(double value)
{
    _type = literals::floatingType;
    addNode(_root, std::to_string(value), _type);
}

void XmlArraySerializer::serializeValueImpl(bool value)
{
    _type = literals::booleanType;
    addNode(_root, value ? literals::booleanTypeTrue : literals::booleanTypeFalse, _type);
}

void XmlArraySerializer::serializeValueImpl(const std::string &value)
{
    _type = literals::stringType;
    addNode(_root, value, _type);
}

void XmlArraySerializer::serializeEmpty()
{
    addNode(_root, "", _type);
}

serialization::ArraySerializer *XmlArraySerializer::serializeArray()
{
    return createStoredInternally<XmlArraySerializer>(addNode(_root));
}

serialization::StructSerializer *XmlArraySerializer::serializeStruct()
{
    return createStoredInternally<CompositeXmlSerializer>(addNode(_root));
}

std::string XmlArraySerializer::dump() const
{
    return dumpXml(_doc.get());
}

xmlNode *XmlArraySerializer::root() const
{
    return _root;
}

// This override does not create new nested array because the root object is
// already an array
serialization::ArraySerializer *RootXmlArraySerializer::serializeArray()
{
    return createStoredInternally<XmlArraySerializer>(root());
}
