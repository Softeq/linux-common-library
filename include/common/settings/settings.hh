/*! \file */
#ifndef SOFTEQ_COMMON_SETTINGS_H
#define SOFTEQ_COMMON_SETTINGS_H

#include <common/stdutils/any.hh>
#include <common/serialization/object_assembler.hh>

#include <map>
#include <memory>
#include <string>
#include <typeindex>
#include <fstream>

namespace softeq
{
namespace common
{
namespace settings
{
/*!
  \brief The list  of functions for settings manipulatation.
*/
class Settings
{
public:
    static Settings &instance();

    template <typename T>
    /*!
      Declaration settings
      Declare T settings, set their default value to reference
      \param[in] reference Type reference
      \param[in] name String name
    */
    void declare(T &reference, const std::string &name)
    {
        declareImpl(reference, name);
    }

    template <typename T, typename std::enable_if<std::is_default_constructible<T>::value, bool>::type = true>
    /*!
      Declaration settings
       \param[in] name String name
    */
    void declare(const std::string &name)
    {
        T ref{};
        declareImpl(ref, name, false);
    }

    /*!
      Undeclaration settings
      \param[in] name String name of settings to undeclare
    */
    void undeclare(const std::string &name)
    {
        if (!isNameDeclared(name))
        {
            throw std::logic_error(common::stdutils::string_format("setting '%s' is not declared", name.c_str()));
        }
        _settings.erase(name);
    }

    template <typename T>
    /*!
      Access to struct node
      \return Node
    */
    T &access()
    {
        for (auto &entry : _settings)
        {
            if (entry.second.value.type() == typeid(T))
            {
                return common::stdutils::any_cast<T>(entry.second.value);
            }
        }
        throw std::logic_error(common::stdutils::string_format("setting '%s' is not found",
                                                               common::stdutils::demangle(typeid(T).name()).c_str()));
    }

    /*!
      Set default serialization and deserialization parameters.
      These parameters are used when serialize and deserialize settings at declaration.

      \param[in] path Path to the file to serialize/deserialize the data at declaration
      \param[in] serializer A unique pointer to serializer will be used by default
      \param[in] deserializer A unique pointer to deserializer will be used by default
    */
    void setDefaultParameters(
            const std::string &path,
            std::unique_ptr<softeq::common::serialization::StructDeserializer> &&deserializer,
            std::unique_ptr<softeq::common::serialization::StructSerializer> &&serializer = nullptr);

    /*!
      Serialize settings data to default file
    */
    void serialize() const;

    /*!
      Deserialize settings from default file
    */
    void deserialize();

    /*!
      Serialize settings data to the file
      \param[in] path A file name for serialized data
      \param[in] serializer A unique pointer to the serializer to use
    */
    void serialize(
            const std::string &path,
            serialization::StructSerializer &serializer) const;

    /*!
      Deserialize settings from the file
      \param[in] path A file name for deserialized data
      \param[in] deserializer A unique pointer to the deserializer to use
    */
    void deserialize(
            const std::string &path,
            serialization::StructDeserializer &deserializer);
    /*!
      Graph
    */
    std::string graph() const;

protected:
    Settings();

private:
    Settings(const Settings &) = delete;
    Settings(Settings &&) = delete;
    Settings &operator=(const Settings &) = delete;
    Settings &operator=(Settings &&) = delete;

    struct BaseValueAssembler
    {
        BaseValueAssembler() = default;
        BaseValueAssembler(const BaseValueAssembler &other) = delete;
        BaseValueAssembler(BaseValueAssembler &&other) = delete;
        BaseValueAssembler &operator=(const BaseValueAssembler &other) = delete;
        BaseValueAssembler &operator=(BaseValueAssembler &&other) = delete;

        virtual ~BaseValueAssembler() = default;

        virtual void serialize(serialization::StructSerializer &serializer, const std::string &name, const common::stdutils::Any &node) const = 0;
        virtual void deserialize(serialization::StructDeserializer &deserializer, const std::string &name, common::stdutils::Any &node) = 0;
        virtual std::string graph(const std::string &assignedNodeName) const = 0;
        virtual std::unique_ptr<BaseValueAssembler> clone() const = 0;
    };

    template <typename TYPE>
    struct ValueAssembler : public BaseValueAssembler
    {
        ValueAssembler(const serialization::ObjectAssembler<TYPE> &assembler)
            : assemblerHolder(assembler)
        {
        }
        ValueAssembler(serialization::ObjectAssembler<TYPE> &&assembler)
            : assemblerHolder(std::move(assembler))
        {
        }
        ValueAssembler(const ValueAssembler &other) = delete;
        ValueAssembler(ValueAssembler &&other) = delete;
        ValueAssembler &operator=(const ValueAssembler &other) = delete;
        ValueAssembler &operator=(ValueAssembler &&other) = delete;

        virtual std::string graph(const std::string &assignedNodeName) const override
        {
            return assemblerHolder.graph(assignedNodeName);
        }

        virtual void serialize(serialization::StructSerializer &serializer, const std::string &name, const common::stdutils::Any &node) const override
        {
            try
            {
                const TYPE &value = common::stdutils::any_cast<TYPE>(node);
                assemblerHolder.serializeElement(serializer, name, value);
            }
            catch (const std::bad_cast &e)
            {
                throw std::logic_error(
                    common::stdutils::string_format("Expected '%s', but node contained %s at serialization",
                                                    common::stdutils::demangle(typeid(TYPE).name()).c_str(),
                                                    common::stdutils::demangle(node.type().name()).c_str()));
            }
        }

        virtual void deserialize(serialization::StructDeserializer &deserializer, const std::string &name, common::stdutils::Any &node) override
        {
            TYPE value;
            assemblerHolder.deserializeElement(deserializer, name, value);
            node = value;
        }

        virtual std::unique_ptr<BaseValueAssembler> clone() const override
        {
            return std::unique_ptr<BaseValueAssembler>(
                new ValueAssembler(serialization::ObjectAssembler<TYPE>::accessor()));
        }

        serialization::ObjectAssembler<TYPE> assemblerHolder;
    };

    struct mapValue_t
    {
        common::stdutils::Any value;
        std::unique_ptr<BaseValueAssembler> assembler;
    };
    friend serialization::ObjectAssembler<std::map<std::string, mapValue_t>>;

    template <typename T>
    bool isTypeDeclared() const
    {
        bool isDeclared = false;
        for (auto &entry : _settings)
        {
            if (entry.second.value.type() == typeid(T))
            {
                isDeclared = true;
                break;
            }
        }

        return isDeclared;
    }

    template <typename T>
    /*!
      Declaration settings private implementation
      Declare T settings, set their default value to reference if they haven't declared
      before or useRefForDeclared is set to true
      \param[in] reference Type reference
      \param[in] name String name
      \param[in] useRefForDeclared substitute already declared parameter with reference
    */
    void declareImpl(T &reference, const std::string &name, bool useRefForDeclared = true)
    {
        if (isTypeDeclared<T>())
        {
            if (useRefForDeclared || !isNameDeclared(name))
            {
                _settings[name].value = reference;
            }
            return;
        }
        else if (isNameDeclared(name))
        {
            throw std::logic_error("setting is already declared: " + name);
        }

        std::unique_ptr<BaseValueAssembler> ptr(new ValueAssembler<T>(serialization::ObjectAssembler<T>::accessor()));
        _settings[name] = mapValue_t{softeq::common::stdutils::Any(reference), std::move(ptr)};

        // try to deserialize a value from a file
        if (!_path.empty() && _deserializer)
        {
            std::ifstream settingsFile(_path);
            // copy value assemblers to stubSettings since mapValue_t has to have an assembler instance to get
            // deserialized
            std::map<std::string, mapValue_t> stubSettings{};
            stubSettings[name] = mapValue_t{common::stdutils::Any(), _settings[name].assembler->clone()};
            deserialize(settingsFile, *_deserializer, stubSettings);

            if (stubSettings[name].value.hasValue())
            {
                if (stubSettings[name].value.type() == typeid(T))
                {
                    _settings[name].value = common::stdutils::Any(stubSettings[name].value);
                }
                else
                {
                    throw std::logic_error("Deserialized structure type mismatch");
                }
            }
        }
    }

    bool isNameDeclared(const std::string &name) const
    {
        return (_settings.find(name) != _settings.end());
    }

    void deserialize(std::ifstream &settingsFileStream,
                     softeq::common::serialization::StructDeserializer &deserializer,
                     std::map<std::string, mapValue_t> &settings);

    std::map<std::string, mapValue_t> _settings;
    std::string _path;
    std::unique_ptr<softeq::common::serialization::StructDeserializer> _deserializer;
    std::unique_ptr<softeq::common::serialization::StructSerializer> _serializer;

public:
    using AccessKey = volatile const int *;
    /*!
      Declaration of a setting from the macros #STATIC_DECLARE_SETTING in the static scope
      Declare T setting, set their default value if they haven't declared before
      ACCESS_KEY is used to make senseless call from other places then macro
      \param[in] name String name of the setting how it will be defined finally in the settings file
      \return Used to prevent the linker to optimize the call because the result will never be used
    */
    template <typename T, Settings::AccessKey ACCESS_KEY>
    static unsigned int __macroOnlyRegistrator(const std::string &name);
};

template <typename T, Settings::AccessKey ACCESS_KEY>
unsigned int Settings::__macroOnlyRegistrator(const std::string &name)
{
    extern volatile const int __settingsAccessKey;
    if (ACCESS_KEY != &__settingsAccessKey)
    {
        throw std::runtime_error("Settings must be registerd by STATIC_DECLARE_SETTING only");
    }

    if (!Settings::instance().isTypeDeclared<T>())
    {
        Settings::instance().declare<T>(name);
    }
    return static_cast<unsigned int>(0xDEADCA11);
}

/*!
 \def STATIC_DECLARE_SETTING
 \param[in] TYPE Declared type
 \param[in] NAME String name of the setting how it will be defined finally in the settings file

 The macro to declare settings in the global memory. It could be useful when you agree with: songleton usage; object
 implementation and usage should be concentrated in a single file
 \snippet settings.cc Static usage motivation
 The macro can be used within softeq::common namespace only
*/
#define STATIC_DECLARE_SETTING(TYPE, NAME)                                                                             \
    extern volatile const int __settingsAccessKey;                                                                              \
    static unsigned int __prevent_placing_macro_within_functions_##TYPE()                                              \
    {                                                                                                                  \
        return softeq::common::settings::Settings::__macroOnlyRegistrator<TYPE, &__settingsAccessKey>(NAME);                   \
    }                                                                                                                  \
    static const unsigned int ___settings__##TYPE = __prevent_placing_macro_within_functions_##TYPE();

} // namespace settings
} // namespace common
} // namespace softeq

#endif // SOFTEQ_COMMON_SETTINGS_H
