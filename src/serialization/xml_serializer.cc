#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <cstdint>
#include <functional>

#include "softeq/common/log.hh"

#include "softeq/common/serialization/xml_serializer.hh"
#include "softeq/common/stdutils.hh"

namespace
{
const char *LOG_DOMAIN = "XmlSerializer";

constexpr const char *cDefaultEncoding = "utf-8";
constexpr const char *cRootNodeName = "root";

using XmlDocUPtr = std::unique_ptr<xmlDoc, std::function<void(xmlDoc *)>>;
using XmlKeyUPtr = std::unique_ptr<xmlChar, std::function<void(xmlChar *)>>;

static void libxml2ErrorHandler(void * /* ctx */, const char *msg, ...)
{
    va_list args;
    va_start(args, msg);
    softeq::common::log().MessageV(softeq::common::LogLevel::ERROR, LOG_DOMAIN, msg, args);
    va_end(args);
}
} // namespace

using namespace softeq::common::serialization;

void XmlSerializer::initMultiThreading()
{
    xmlInitParser();
}

void XmlSerializer::cleanup()
{
    xmlCleanupParser();
}

XmlSerializer::XmlSerializer()
{
    // According to the libxml2 description, the generic error function should
    // be set for each thread separately. The libxml2ErrorHandler is reenterable.
    xmlSetGenericErrorFunc(nullptr, libxml2ErrorHandler);
}

XmlSerializer::~XmlSerializer()
{
    // The deleters calls sequence hasn't to be violated !
    _doc.reset();
    _writer.reset();
    _buffer.reset();
}

std::string XmlSerializer::dump() const
{
    std::string result;
    if (_buffer != nullptr)
    {
        result = std::string(reinterpret_cast<char *>(_buffer->content));
    }
    else if (_doc != nullptr)
    {
        xmlChar *str = nullptr;
        int size = 0;
        xmlDocDumpMemory(_doc.get(), &str, &size);
        if (str != nullptr)
        {
            XmlCharUP strPtr(str, [](xmlChar *ptr) { xmlFree(ptr); });
            result = std::string(reinterpret_cast<char *>(strPtr.get()));
        }
    }

    return result;
}

bool XmlSerializer::hasNamedNode() const
{
    assert(!_frames.empty());
    assert(_frames.top().node != nullptr);
    assert(_name.hasValue());
    return (getChildNodeByName(*(_frames.top().node), _name.cValue()) != nullptr);
}

XmlSerializer::CurrentObjectType XmlSerializer::getCurrentObjectType() const
{
    assert(!_frames.empty());
    return _frames.top().type;
}

XmlSerializer::operator std::string() const
{
    return dump();
}

const std::vector<std::string> XmlSerializer::getNodeKeys() const
{
    assert(!_frames.empty());
    assert(_frames.top().node != nullptr);

    std::vector<std::string> result;

    xmlNode *currentNode = _frames.top().node->xmlChildrenNode;

    while (currentNode != nullptr)
    {
        if (currentNode->type == XML_ELEMENT_NODE)
        {
            std::string name = reinterpret_cast<const char *>(currentNode->name);
            if (name == cConatainerEntry)
            {
                throw std::runtime_error("XML node type is array. Not struct");
            }
            result.push_back(name);
        }
        currentNode = currentNode->next;
    }

    return result;
}

void XmlSerializer::name(const std::string &name)
{
    _name = name;
}

void XmlSerializer::beginObjectSerialization(CurrentObjectType type)
{
    Frame frame = {type};

    if (_frames.empty())
    {
        // Begin serialization. Create root node.
        assert(!_name.hasValue());

        _buffer = XmlBufferUP(xmlBufferCreate(), [](xmlBuffer *ptr) { xmlBufferFree(ptr); });
        xmlBufferSetAllocationScheme(_buffer.get(), XML_BUFFER_ALLOC_DOUBLEIT);
        assert(_buffer != nullptr);

        _writer = XmlTextWriterUP(xmlNewTextWriterMemory(_buffer.get(), false),
                                  [](xmlTextWriter *ptr) { xmlFreeTextWriter(ptr); });
        assert(_writer != nullptr);

        int rc = xmlTextWriterStartDocument(_writer.get(), nullptr, cDefaultEncoding, nullptr);
        assert(rc >= 0);

        rc = xmlTextWriterStartElement(_writer.get(), reinterpret_cast<const xmlChar *>(cRootNodeName));
        assert(rc >= 0);
    }
    else
    {
        // Create child struct/array node
        assert(_name.hasValue());
        if (_frames.top().type == CurrentObjectType::ARRAY)
        {
            int rc = xmlTextWriterStartElement(_writer.get(), reinterpret_cast<const xmlChar *>(cConatainerEntry));
            assert(rc >= 0);
        }
        else
        {
            int rc =
                xmlTextWriterStartElement(_writer.get(), reinterpret_cast<const xmlChar *>(_name.cValue().c_str()));
            assert(rc >= 0);
        }
    }

    _frames.push(frame);
}

void XmlSerializer::valuePrimitive(bool v)
{
    assert(_name.hasValue());
    assert(!_frames.empty());

    std::string nodeName;
    if (_frames.top().type == CurrentObjectType::ARRAY)
    {
        nodeName = cConatainerEntry;
    }
    else
    {
        nodeName = _name;
    }

    // Create a node for a serialized primitive
    int rc = xmlTextWriterStartElement(_writer.get(), reinterpret_cast<const xmlChar *>(nodeName.c_str()));
    assert(rc >= 0);
    valueSet(v);
    rc = xmlTextWriterEndElement(_writer.get());
    assert(rc >= 0);
}

void XmlSerializer::valuePrimitive(int64_t v)
{
    valuePrimitiveSet(v);
}

void XmlSerializer::valuePrimitive(uint64_t v)
{
    valuePrimitiveSet(v);
}

void XmlSerializer::valuePrimitive(double v)
{
    valuePrimitiveSet(v);
}

void XmlSerializer::valuePrimitive(const std::string &v)
{
    valuePrimitiveSet(v);
}

void XmlSerializer::endObjectSerialization()
{
    int rc = xmlTextWriterEndElement(_writer.get());
    assert(rc >= 0);
    assert(!_frames.empty());
    _frames.pop();

    if (_frames.empty())
    {
        rc = xmlTextWriterEndDocument(_writer.get());
        assert(rc >= 0);

        _name = Optional<std::string>();
    }
}

size_t XmlSerializer::beginObjectDeserialization(CurrentObjectType type)
{
    xmlNode *frameNode = nullptr;
    if (_frames.empty())
    {
        // Begin deserialization. Find root node and push it on the top of the stack.
        assert(type == CurrentObjectType::STRUCT);
        assert(_doc != nullptr);
        frameNode = xmlDocGetRootElement(_doc.get());
        if (frameNode == nullptr)
        {
            throw NullNodeException("Cannot get root node");
        }
    }
    else
    {
        assert(!_frames.empty());
        assert(_frames.top().node != nullptr);
        // Find a chicd node for a deserialized struct/array
        if (_frames.top().type == CurrentObjectType::ARRAY)
        {
            frameNode = getChildNodeByIndex(*(_frames.top().node), _frames.top().index);

            if (frameNode == nullptr)
            {
                throw NullNodeException("Cannot find child node by index " + std::to_string(_frames.top().index));
            }
            else if (reinterpret_cast<const char *>(frameNode->name) != std::string(cConatainerEntry))
            {
                throw ParseException(std::string("Not container node. Node name '") +
                                     reinterpret_cast<const char *>(frameNode->name) + "'");
            }

            // Current array contains objects. Increment its index.
            _frames.top().index++;
        }
        else
        {
            assert(_name.hasValue());
            frameNode = getChildNodeByName(*(_frames.top().node), _name.cValue());
            if (frameNode == nullptr)
            {
                throw NullNodeException("Cannot find child node by name '" + _name.cValue() + "'");
            }
        }
    }

    _frames.push({type, frameNode});
    return getNumberOfChildNodes(*frameNode);
}

// Deserializes primitives
softeq::common::Any XmlSerializer::value()
{
    softeq::common::Any any;
    assert(!_frames.empty());
    xmlNode *node;

    // Frame contains a pointer to a struct/container node.
    // Find an appropriate primitive child node to deserialize.
    if (_frames.top().type == CurrentObjectType::ARRAY)
    {
        assert(_frames.top().node != nullptr);
        node = getChildNodeByIndex(*(_frames.top().node), _frames.top().index);
        if (node == nullptr)
        {
            throw NullNodeException(std::string("Null node '") + _name.cValue() +
                                    "' for primitive. Index=" + std::to_string(_frames.top().index));
        }
        else if (reinterpret_cast<const char *>(node->name) != std::string(cConatainerEntry))
        {
            throw ParseException(std::string("Not container node. Node name '") +
                                 reinterpret_cast<const char *>(node->name) + "'");
        }

        // Current array contains primitives. Increment its index.
        _frames.top().index++;
    }
    else
    {
        assert(_name.hasValue());
        node = getChildNodeByName(*(_frames.top().node), _name);
        if (node == nullptr)
        {
            throw NullNodeException(std::string("Null node '") + _name.cValue() + "' for primitive");
        }
    }

    XmlCharUP typeProperty(xmlGetProp(node, reinterpret_cast<const xmlChar *>(cTypeProperty)),
                           [](xmlChar *ptr) { xmlFree(ptr); });
    if (typeProperty == nullptr)
    {
        throw ParseException("Empty type attribute in '" + _name.cValue() + "'");
    }
    const std::string type = reinterpret_cast<char *>(typeProperty.get());

    node = node->children;
    std::string content;
    if (node != nullptr)
    {
        if (node->type != XML_TEXT_NODE)
        {
            throw ParseException("Not XML_TEXT_NODE. Node type: " + std::to_string(node->type));
        }

        XmlCharUP rawContent(xmlNodeListGetString(_doc.get(), node, 1), [](xmlChar *ptr) { xmlFree(ptr); });
        if (rawContent == nullptr)
        {
            throw ParseException("Empty content");
        }
        content = reinterpret_cast<char *>(rawContent.get());
    }
    else
    {
        content = "";
    }

    // Deserializer knows nothing about requested type.
    // Thus, it uses as wide type as possible.
    if (type == cSignedIntegerType)
    {
        try
        {
            int64_t value = std::stol(content);
            any = value;
        }
        catch (const std::exception &e)
        {
            throw ParseException(stdutils::string_format("Wrong string for conversion to signed integer type. %s", e.what()));
        }
    }
    else if (type == cUnsignedIntegerType)
    {
        try
        {
            // if content starts with '-', handle as signed type to prevent wraparound by std::stoul
            if ((content.size() > 1) && (content[0] == '-'))
            {
                int64_t value = std::stol(content);
                any = value;
            }
            else
            {
                uint64_t value = std::stoul(content);
                any = value;
            }
        }
        catch (const std::exception &e)
        {
            throw ParseException(stdutils::string_format("Wrong string for conversion to unsigned integer type. %s", e.what()));
        }
    }
    else if (type == cFloatingType)
    {
        try
        {
            double value = std::stod(content);
            any = value;
        }
        catch (const std::exception &e)
        {
            throw ParseException(stdutils::string_format("Wrong string for conversion to floating point type. %s", e.what()));
        }
    }
    else if (type == cBooleanType)
    {
        if (content == cBooleanTypeTrue)
        {
            any = true;
        }
        else if (content == cBooleanTypeFalse)
        {
            any = false;
        }
        else
        {
            throw ParseException("Wrong boolean value '" + content + "' in '" + _name.cValue() + "'");
        }
    }
    else if (type == cStringType)
    {
        any = content;
    }
    else
    {
        throw ParseException("Not supported primitive type '" + type + "'");
    }

    return any;
}

void XmlSerializer::endObjectDeserialization()
{
    assert(!_frames.empty());
    _frames.pop();

    if (_frames.empty())
    {
        _name = Optional<std::string>();
    }
}

void XmlSerializer::setRawInput(const std::string &input)
{
    _doc =
        XmlDocUP(xmlReadMemory(input.c_str(), input.size(), nullptr, nullptr, 0), [](xmlDoc *ptr) { xmlFreeDoc(ptr); });
    if (_doc == nullptr)
    {
        throw ParseException("xml parsing error \n" + input);
    }
}

size_t XmlSerializer::getNumberOfChildNodes(const xmlNode &node) const
{
    size_t size = 0;
    xmlNode *currentNode = node.xmlChildrenNode;
    while (currentNode != nullptr)
    {
        if (currentNode->type == XML_ELEMENT_NODE)
        {
            size++;
        }
        currentNode = currentNode->next;
    }

    return size;
}

xmlNode *XmlSerializer::getChildNodeByName(const xmlNode &node, const std::string &nodeName) const
{
    xmlNode *foundNode = nullptr;
    xmlNode *currentNode = node.xmlChildrenNode;
    while (currentNode != nullptr)
    {
        if (xmlStrcmp(currentNode->name, reinterpret_cast<const xmlChar *>(nodeName.c_str())) == 0)
        {
            foundNode = currentNode;
            assert(foundNode->type == XML_ELEMENT_NODE);
            break;
        }
        currentNode = currentNode->next;
    }

    return foundNode;
}

xmlNode *XmlSerializer::getChildNodeByIndex(const xmlNode &node, size_t index) const
{
    xmlNode *foundNode = nullptr;
    xmlNode *currentNode = node.xmlChildrenNode;

    size_t count = 0;
    while (currentNode != nullptr)
    {
        if (currentNode->type == XML_ELEMENT_NODE)
        {
            if (index == count)
            {
                foundNode = currentNode;
                break;
            }
            count++;
        }
        currentNode = currentNode->next;
    }

    return foundNode;
}
