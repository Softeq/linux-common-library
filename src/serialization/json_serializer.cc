#include <cstdint>
#include <sstream>

#include "softeq/common/log.hh"
#include "softeq/common/serialization/json_serializer.hh"

using namespace softeq::common::serialization;

void JSonSerializer::name(const std::string &name)
{
    _name = name;
}

void JSonSerializer::beginObjectSerialization(CurrentObjectType type)
{
    Frame frame = {type, nullptr, 0};
    if (_frames.empty())
    {
        frame.node = &_rootJson;
        *(frame.node) = nlohmann::json::object();
    }

    if (_name.hasValue()) // only root object does not have name
    {
        assert(!_frames.empty());
        assert(_frames.top().node != nullptr);
        if (type == CurrentObjectType::ARRAY)
        {
            // array in array
            if (_frames.top().type == CurrentObjectType::ARRAY)
            {
                assert(_frames.top().node->is_array());
                _frames.top().node->push_back(nlohmann::json::array()); // add empty array into current array node
                frame.node = &(_frames.top().node->back());             // to push added array on top
            }
            // array in object
            else
            {
                assert(_frames.top().node->is_object() || frame.node->is_null());
                (*(_frames.top().node))[_name] = nlohmann::json::array(); // add array into current node
                frame.node = &((*(_frames.top().node))[_name]);           // to push added array on top
            }
        }
        else // OBJECT
        {
            // object in array
            if (_frames.top().type == CurrentObjectType::ARRAY)
            {
                assert(_frames.top().node->is_array());
                _frames.top().node->push_back({});          // add empty object into current array node
                frame.node = &(_frames.top().node->back()); // to push added struct on top
            }
            // object in object
            else
            {
                assert(_frames.top().node->is_object() || frame.node->is_null());
                (*(_frames.top().node))[_name] = nlohmann::json::object(); // add object into current node
                frame.node = &((*(_frames.top().node))[_name]);            // to push added object on top
            }
        }
    }

    assert(frame.node != nullptr);
    _frames.push(frame);
}

void JSonSerializer::valuePrimitive(bool v)
{
    valueSet(v);
}
void JSonSerializer::valuePrimitive(int64_t v)
{
    valueSet(v);
}
void JSonSerializer::valuePrimitive(uint64_t v)
{
    valueSet(v);
}
void JSonSerializer::valuePrimitive(double v)
{
    valueSet(v);
}
void JSonSerializer::valuePrimitive(const std::string &v)
{
    valueSet(v);
}

void JSonSerializer::endObjectSerialization()
{
    assert(!_frames.empty());
    _frames.pop();

    if (_frames.empty())
    {
        _name = Optional<std::string>();
    }
}

size_t JSonSerializer::beginObjectDeserialization(CurrentObjectType type)
{
    Frame frame = {type, nullptr, 0};
    if (_frames.empty())
    {
        frame.node = &_rootJson;
    }

    if (_name.hasValue()) // only root object does not have name
    {
        assert(!_frames.empty());
        assert(_frames.top().node != nullptr);
        // Select appropriate object/array if new object/array is inside array
        if (_frames.top().type == CurrentObjectType::ARRAY)
        {
            if (!_frames.top().node->is_array())
            {
                throw ParseException("Node is not array");
            }
            size_t &index = _frames.top().index;
            frame.node = &(_frames.top().node->at(index));
            // Current array contains objects. Increment its index.
            index++;
        }
        // Select object/array by name if new object/array is inside another object
        else if (_frames.top().type == CurrentObjectType::STRUCT)
        {
            if (!_frames.top().node->is_object())
            {
                throw ParseException("Node is not object");
            }
            frame.node = &((*(_frames.top().node))[_name]);
        }
        else
        {
            assert(((void)"not supported type", 0));
        }
    }

    assert(frame.node != nullptr);
    if (frame.node->is_null())
    {
        throw NullNodeException(std::string("Null node for object '") + _name.cValue() + "'");
    }
    else if (frame.node->is_array() && (type != CurrentObjectType::ARRAY))
    {
        throw ParseException("STRUCT node, but ARRAY is expected");
    }
    else if (frame.node->is_object() && (type != CurrentObjectType::STRUCT))
    {
        throw ParseException("ARRAY node, but STRUCT is expected");
    }

    _frames.push(frame);
    return frame.node->size(); // number of nodes in current node
}

softeq::common::Any JSonSerializer::value()
{
    Frame &frame = _frames.top();
    assert(_name.hasValue());

    assert(frame.node != nullptr);
    nlohmann::json *node;
    if (frame.type == CurrentObjectType::ARRAY)
    {
        if (!frame.node->is_array())
        {
            throw ParseException("Node is not array");
        }
        assert(frame.index < frame.node->size());
        node = &(frame.node->at(frame.index));

        // Current array is array of primitives. Increment its index.
        frame.index++;
    }
    else
    {
        if (!frame.node->is_object())
        {
            throw ParseException("Node is not object");
        }
        node = &((*(frame.node))[_name]);
    }
    assert(node != nullptr);
    if (!node->is_primitive())
    {
        throw ParseException("Node " + _name.cValue() + " is not primitive");
    }

    // Deserializer knows nothing about requested type.
    // Thus, it uses as wide type as possible.
    softeq::common::Any any;
    switch (node->type())
    {
    case nlohmann::detail::value_t::boolean:
        any = node->get<bool>();
        break;
    case nlohmann::detail::value_t::number_unsigned:
        any = node->get<uint64_t>();
        break;
    case nlohmann::detail::value_t::number_integer:
        any = node->get<int64_t>();
        break;
    case nlohmann::detail::value_t::number_float:
        any = node->get<double>();
        break;
    case nlohmann::detail::value_t::string:
        any = node->get<std::string>();
        break;
    case nlohmann::detail::value_t::null:
        throw NullNodeException(std::string("Null node for primitive '") + _name.cValue() + "'");
    default:
        assert(((void)"not supported primitive", 0));
        break;
    }
    return any;
}

void JSonSerializer::endObjectDeserialization()
{
    assert(!_frames.empty());
    _frames.pop();

    if (_frames.empty())
    {
        _name = Optional<std::string>();
        _rootJson = nlohmann::json::object();
    }
}

std::string JSonSerializer::dump() const
{
    return _rootJson.dump();
}

void JSonSerializer::setRawInput(const std::string &json)
{
    try
    {
        _rootJson.clear();
        std::stringstream ss;
        ss << json;
        ss >> _rootJson;
    }
    catch (const nlohmann::json::parse_error &e)
    {
        throw ParseException(e.what());
    }
}

bool JSonSerializer::hasNamedNode() const
{
    assert(!_frames.empty());
    assert(_frames.top().node != nullptr);
    assert(_name.hasValue());
    return _frames.top().node->find(_name) != _frames.top().node->end();
}

JSonSerializer::CurrentObjectType JSonSerializer::getCurrentObjectType() const
{
    assert(!_frames.empty());
    return _frames.top().type;
}

JSonSerializer::operator std::string() const
{
    return dump();
}

const std::vector<std::string> JSonSerializer::getNodeKeys() const
{
    assert(!_frames.empty());
    assert(_frames.top().node != nullptr);
    if (!_frames.top().node->is_object())
    {
        throw ParseException("Json node type is not object");
    }
    std::vector<std::string> result;

    auto *node = _frames.top().node;
    for (nlohmann::json::iterator it = node->begin(); it != node->end(); it++)
    {
        result.push_back(it.key());
    }

    return result;
}
