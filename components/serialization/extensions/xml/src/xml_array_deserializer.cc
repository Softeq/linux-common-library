#include "xml_array_deserializer.hh"
#include "xml_struct_deserializer.hh"
#include "xml_utils.hh"

namespace softeq
{
namespace common
{
namespace serialization
{
namespace xml
{
namespace
{
// Checks if the node's name is literals::containerEntry, throws otherwise
void validateEntryNode(xmlNode *node)
{
    if (xmlCharToString(node->name) != literals::containerEntry)
    {
        throw ParseException(xmlCharToString(node->name), "wrong array container entry format");
    }
}

} // namespace

void XmlArrayDeserializer::setRawInput(const std::string &input)
{
    _doc.reset(xmlReadMemory(input.c_str(), static_cast<int>(input.size()), nullptr, nullptr, 0));
    if (_doc == nullptr)
    {
        throw ParseException("", xmlGetLastError()->message);
    }

    _node = xmlDocGetRootElement(_doc.get());
}

XmlArrayDeserializer::XmlArrayDeserializer(XmlDocSP doc, xmlNode *node)
    : _doc(doc)
    , _node(node->xmlChildrenNode)
{
}

stdutils::Any XmlArrayDeserializer::value()
{
    if (_node)
    {
        validateEntryNode(_node);
        auto value = nodeValue(_doc.get(), _node);
        _node = _node->next;
        ++_index;
        return value;
    }
    return stdutils::Any();
}

std::size_t XmlArrayDeserializer::index() const
{
    return _index;
}

serialization::StructDeserializer *XmlArrayDeserializer::deserializeStruct()
{
    if (_node)
    {
        validateEntryNode(_node);
        auto serializer = createStoredInternally<CompositeXmlDeserializer>(_doc, _node);
        _node = _node->next;
        ++_index;
        return serializer;
    }
    return nullptr;
}

serialization::ArrayDeserializer *XmlArrayDeserializer::deserializeArray()
{
    if (_node)
    {
        validateEntryNode(_node);
        auto deserializer = createStoredInternally<XmlArrayDeserializer>(_doc, _node);
        _node = _node->next;
        ++_index;
        return deserializer;
    }
    return nullptr;
}

bool XmlArrayDeserializer::isComplete() const
{
    return _node == nullptr;
}

bool XmlArrayDeserializer::nextValueExists() const
{
    return isComplete() ? false : !isEmptyNode(_node);
}

XmlDocSP XmlArrayDeserializer::doc() const
{
    return _doc;
}

xmlNode *XmlArrayDeserializer::element() const
{
    return _node;
}

serialization::ArrayDeserializer *RootXmlArrayDeserializer::deserializeArray()
{
    return createStoredInternally<XmlArrayDeserializer>(doc(), element());
}

} // namespace xml
} // namespace serialization
} // namespace common
} // namespace softeq
