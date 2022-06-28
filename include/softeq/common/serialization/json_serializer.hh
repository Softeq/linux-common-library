#ifndef SOFTEQ_COMMON_SERIALIZATION_JSON_SERIALIZER_H
#define SOFTEQ_COMMON_SERIALIZATION_JSON_SERIALIZER_H

#include <assert.h>
#include <stack>
#include <string>
#include <typeinfo>

#include "nlohmann/json.hpp"

#include "softeq/common/any.hh"
#include "softeq/common/optional.hh"

#include "softeq/common/serialization/serializer.hh"

namespace softeq
{
namespace common
{
namespace serialization
{

class JSonSerializer : public Serializer
{
public:
    JSonSerializer() = default;

    std::string dump() const override;
    bool hasNamedNode() const override;
    CurrentObjectType getCurrentObjectType() const override;
    const std::vector<std::string> getNodeKeys() const override;
    void setRawInput(const std::string &input) override;
    operator std::string() const override;

    void name(const std::string &name) override;
    void beginObjectSerialization(CurrentObjectType type) override;
    void endObjectSerialization() override;

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
    struct Frame
    {
        CurrentObjectType type;
        nlohmann::json *node;
        size_t index;
    };

    Optional<std::string> _name;
    nlohmann::json _rootJson;
    std::stack<Frame> _frames;

    template <typename T>
    void valueSet(T &v)
    {
        assert(!_frames.empty());

        Frame &frame = _frames.top();
        assert(frame.node != nullptr);
        if (frame.type == CurrentObjectType::ARRAY)
        {
            assert(frame.node->is_array());
            frame.node->push_back(v);
            assert(frame.node->back().is_primitive());
        }
        else
        {
            assert(_name.hasValue());
            assert(frame.node->is_object() || frame.node->is_null());
            (*(frame.node))[_name] = v;
            assert((*(frame.node))[_name].is_primitive());
        }
    }
};

} // namespace serialization
} // namespace common
} // namespace softeq

#endif // SOFTEQ_COMMON_SERIALIZATION_JSON_SERIALIZER_H