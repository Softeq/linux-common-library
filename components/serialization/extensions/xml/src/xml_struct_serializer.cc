#include "xml_struct_serializer.hh"
#include "xml_array_serializer.hh"

using namespace softeq::common;
using namespace softeq::common::serialization::xml;

CompositeXmlSerializer::CompositeXmlSerializer(xmlNode *node)
    : _root(node)
{
}

CompositeXmlSerializer::CompositeXmlSerializer()
{
    // we can call xmlSetGenericErrorFunc(nullptr, libxml2ErrorHandler) to hide libxml2 output to the console
    _doc.reset(xmlNewDoc(nullptr));
    if (!_doc) // it can fail if allocation fails
    {
        throw std::bad_alloc();
    }
    _root = xmlNewDocNode(_doc.get(), nullptr, stringToXmlChar(literals::rootNodeName), nullptr);
    xmlDocSetRootElement(_doc.get(), _root);
}

namespace
{
void addNode(xmlNode *node, const std::string &name, const std::string &value, const std::string &type)
{
    xmlNode *child = xmlNewChild(node, nullptr, stringToXmlChar(name.c_str()), nullptr);
    xmlNodeAddContent(child, stringToXmlChar(value.c_str()));
    xmlNewProp(child, stringToXmlChar(literals::typeProperty), stringToXmlChar(type.c_str()));
}
xmlNode *addNode(xmlNode *node, const std::string &name)
{
    return xmlNewChild(node, nullptr, stringToXmlChar(name.c_str()), nullptr);
}

} // namespace

void CompositeXmlSerializer::serializeValueImpl(const std::string &name, int64_t value)
{
    addNode(_root, name, std::to_string(value), literals::signedIntegerType);
}

void CompositeXmlSerializer::serializeValueImpl(const std::string &name, uint64_t value)
{
    addNode(_root, name, std::to_string(value), literals::unsignedIntegerType);
}

void CompositeXmlSerializer::serializeValueImpl(const std::string &name, const std::string &value)
{
    addNode(_root, name, value, literals::stringType);
}

void CompositeXmlSerializer::serializeValueImpl(const std::string &name, double value)
{
    addNode(_root, name, std::to_string(value), literals::floatingType);
}

void CompositeXmlSerializer::serializeValueImpl(const std::string &name, bool value)
{
    addNode(_root, name, value ? literals::booleanTypeTrue : literals::booleanTypeFalse, literals::booleanType);
}

serialization::StructSerializer *CompositeXmlSerializer::serializeStruct(const std::string &name)
{
    return createStoredInternally<CompositeXmlSerializer>(addNode(_root, name));
}

serialization::ArraySerializer *CompositeXmlSerializer::serializeArray(const std::string &name)
{
    return createStoredInternally<XmlArraySerializer>(addNode(_root, name));
}

std::string CompositeXmlSerializer::dump() const
{
    return dumpXml(_doc.get());
}
