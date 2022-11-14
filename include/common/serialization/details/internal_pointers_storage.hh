#ifndef SOFTEQ_COMMON_SERIALIZATION_UTILS_POINTER_STORAGE_H
#define SOFTEQ_COMMON_SERIALIZATION_UTILS_POINTER_STORAGE_H

#include <vector>
#include <memory>

namespace softeq
{
namespace common
{
namespace serialization
{
/*
 * When the serialization works it needs to store pointers of internal subobjects and arrays. This class implements just
 * a trick how to do it common way for Serializable and Deserializable. This is needed because we do not want to share
 * ownership of objects to the upper level but want to provide ability to control each level of abstraction.
 * This can be used when it is needed to implement different types of serialization INI in XML, JSon in INI and
 * especially Binary in Base64
 */
template  <typename BaseTypeToStore>
class ContainsInternalPointersStorage
{
protected:
    template <typename InheritedType, typename... Args>
    InheritedType *createStoredInternally(Args&&... deserializerArgs)
    {
        std::unique_ptr<InheritedType> objectToStore(new InheritedType(std::forward<Args>(deserializerArgs)...));
        InheritedType *nonOwningPointerForClients = objectToStore.get();
        _pointersStorage.push_back(std::move(objectToStore));
        return nonOwningPointerForClients;
    }

private:
    /*
     * contains pointers to de/serializers of each field of an object
     */
    std::vector<std::unique_ptr<BaseTypeToStore>> _pointersStorage;
};

} // namespace serialization
} // namespace common
} // namespace softeq

#endif // SOFTEQ_COMMON_SERIALIZATION_UTILS_POINTER_STORAGE_H
