#include "xml_struct_deserializer.hh"
#include "xml_array_deserializer.hh"
#include "xml_utils.hh"

#include <algorithm>

#include <iostream>

using namespace softeq::common;
using namespace softeq::common::serialization;
using namespace softeq::common::serialization::xml;

void CompositeXmlDeserializer::initMultiThreading()
{
    xmlInitParser();
}

void CompositeXmlDeserializer::cleanup()
{
    xmlCleanupParser();
}

void CompositeXmlDeserializer::setRawInput(const std::string &input)
{
    _doc.reset(xmlReadMemory(input.c_str(), static_cast<int>(input.size()), nullptr, nullptr, 0), XmlDocDeleter());
    if (_doc == nullptr)
    {
        throw ParseException("", xmlGetLastError()->message);
    }

    _nodes = discoverNodes(xmlDocGetRootElement(_doc.get()));
}

CompositeXmlDeserializer::CompositeXmlDeserializer(XmlDocSP doc, const xmlNode *node)
    : _doc(doc)
{
    _nodes = discoverNodes(node);
}

bool CompositeXmlDeserializer::valueExists(const std::string &name) const
{
    return _nodes.count(name) == 1;
}

std::vector<std::string> CompositeXmlDeserializer::availableNames() const
{
    std::vector<std::string> names;
    std::transform(std::begin(_nodes), std::end(_nodes), std::back_inserter(names),
                   [](const std::pair<std::string, xmlNode *> &item) { return item.first; });
    return names;
}

stdutils::Any CompositeXmlDeserializer::value(const std::string &name)
{
    return nodeValue(_doc.get(), _nodes.at(name));
}

StructDeserializer *CompositeXmlDeserializer::deserializeStruct(const std::string &name)
{
    const auto element = _nodes.find(name);
    if (element != std::end(_nodes))
    {
        return createStoredInternally<CompositeXmlDeserializer>(_doc, element->second);
    }
    return nullptr;
}

ArrayDeserializer *CompositeXmlDeserializer::deserializeArray(const std::string &name)
{
    const auto element = _nodes.find(name);
    if (element != std::end(_nodes))
    {
        return createStoredInternally<XmlArrayDeserializer>(_doc, element->second);
    }
    return nullptr;
}
