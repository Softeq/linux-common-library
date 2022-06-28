#ifndef SOFTEQ_COMMON_SERIALIZATION_XML_SERIALIZER_H
#define SOFTEQ_COMMON_SERIALIZATION_XML_SERIALIZER_H

#include <cassert>
#include <libxml/parser.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <list>
#include <memory>
#include <mutex>
#include <stack>
#include <string>
#include <type_traits>

#include "softeq/common/any.hh"
#include "softeq/common/optional.hh"
#include "softeq/common/serialization/serializer.hh"

namespace softeq
{
namespace common
{
namespace serialization
{
class XmlSerializer : public Serializer
{
public:
    /*!
       Initialization function for the XML parser of Libxm2.
       The function should be called once before processing
       in case of use in multithreaded programs. It is not
       nessessary to call it in singlethreaded programs.
       NB: THE FUNCION HAS TO BE CALLED FROM THE MAIN THREAD.
    */
    static void initMultiThreading();

    /*!
       The function cleans up memory allocated be the Libxml2 library itself.
       The function should be called after ALL INSTANCES of XmlSerializer ARE
       DELETED both for a signle and a multithreaded programs.
       NB: THE FUNCION HAS TO BE CALLED FROM THE MAIN THREAD IN CASE OF
       MULTITHREADED PROGRAMS.
    */
    static void cleanup();

    XmlSerializer();
    ~XmlSerializer();

    std::string dump() const override;
    bool hasNamedNode() const override;
    CurrentObjectType getCurrentObjectType() const override;
    const std::vector<std::string> getNodeKeys() const override;
    void setRawInput(const std::string &input) override;
    operator std::string() const override;

    void name(const std::string &name) override;
    void beginObjectSerialization(CurrentObjectType type) override;
    void endObjectSerialization() override;
    // Function returns number of elements in node.
    // For objects it returns number of children nodes.
    // For containers it returns number of elements in a container.
    size_t beginObjectDeserialization(CurrentObjectType type) override;
    Any value() override;
    void endObjectDeserialization() override;

protected:
    void valuePrimitive(bool v) override;
    void valuePrimitive(int64_t v) override;
    void valuePrimitive(uint64_t v) override;
    void valuePrimitive(double v) override;
    void valuePrimitive(const std::string &v) override;

private:
    constexpr static const char *cTypeProperty = "type";
    constexpr static const char *cSignedIntegerType = "int";
    constexpr static const char *cUnsignedIntegerType = "uint";
    constexpr static const char *cFloatingType = "float";
    constexpr static const char *cStringType = "string";
    constexpr static const char *cBooleanType = "bool";
    constexpr static const char *cBooleanTypeTrue = "true";
    constexpr static const char *cBooleanTypeFalse = "false";
    constexpr static const char *cConatainerEntry = "___containerEntry___";

    // A frame describes a tree node of an object (struct or container, not primitives)
    // Access to a primitive node within an object is performed by means of value()
    // where a node of a primitive is selected to read/write.
    struct Frame
    {
        // Type of a frame
        CurrentObjectType type;

        // Points to an object node (struct or container)
        xmlNode *node;

        // index is used to iterate over containers at serialization.
        // It is incremented when a value from a container is received.
        size_t index;
    };
    // Stack is used to go over tree when children nodes need to be accessed
    // and have an ability to get a parent node back.
    std::stack<Frame> _frames;

    using XmlTextWriterUP = std::unique_ptr<xmlTextWriter, std::function<void(xmlTextWriter *)>>;
    using XmlBufferUP = std::unique_ptr<xmlBuffer, std::function<void(xmlBuffer *)>>;
    using XmlDocUP = std::unique_ptr<xmlDoc, std::function<void(xmlDoc *)>>;
    using XmlCharUP = std::unique_ptr<xmlChar, std::function<void(xmlChar *)>>;

    XmlTextWriterUP _writer;
    XmlBufferUP _buffer;
    XmlDocUP _doc;

    Optional<std::string> _name;

    size_t getNumberOfChildNodes(const xmlNode &node) const;
    xmlNode *getChildNodeByName(const xmlNode &node, const std::string &nodeName) const;
    xmlNode *getChildNodeByIndex(const xmlNode &node, size_t index) const;

    template <typename T>
    void valuePrimitiveSet(const T &v)
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

    template <typename T,
              typename std::enable_if<std::is_arithmetic<T>::value && !std::is_same<T, bool>::value, int>::type = 0>
    void valueSet(const T &v)
    {
        int rc;
        const char *type = nullptr;
        if (std::is_integral<T>::value && std::is_signed<T>::value)
        {
            type = cSignedIntegerType;
        }
        else if (std::is_integral<T>::value && std::is_unsigned<T>::value)
        {
            type = cUnsignedIntegerType;
        }
        else if (std::is_floating_point<T>::value)
        {
            type = cFloatingType;
        }
        else
        {
            assert(0);
        }
        assert(type != nullptr);

        rc = xmlTextWriterWriteAttribute(_writer.get(), reinterpret_cast<const xmlChar *>(cTypeProperty),
                                         reinterpret_cast<const xmlChar *>(type));
        assert(rc >= 0);

        rc = xmlTextWriterWriteRaw(_writer.get(), reinterpret_cast<const xmlChar *>(std::to_string(v).c_str()));
        assert(rc >= 0);
    }

    template <typename T, typename std::enable_if<std::is_same<T, std::string>::value, int>::type = 0>
    void valueSet(const T &v)
    {
        int rc = xmlTextWriterWriteAttribute(_writer.get(), reinterpret_cast<const xmlChar *>(cTypeProperty),
                                             reinterpret_cast<const xmlChar *>(cStringType));
        assert(rc >= 0);

        rc = xmlTextWriterWriteRaw(_writer.get(), reinterpret_cast<const xmlChar *>(v.c_str()));
        assert(rc >= 0);
    }

    template <typename T, typename std::enable_if<std::is_same<T, bool>::value, int>::type = 0>
    void valueSet(const T &v)
    {
        int rc = xmlTextWriterWriteAttribute(_writer.get(), reinterpret_cast<const xmlChar *>(cTypeProperty),
                                             reinterpret_cast<const xmlChar *>(cBooleanType));
        assert(rc >= 0);

        rc = xmlTextWriterWriteRaw(
            _writer.get(), reinterpret_cast<const xmlChar *>((v == false) ? cBooleanTypeFalse : cBooleanTypeTrue));
        assert(rc >= 0);
    }
};

} // namespace serialization
} // namespace common
} // namespace softeq

#endif // SOFTEQ_COMMON_SERIALIZATION_XML_SERIALIZER_H