/*! \file */
#ifndef SOFTEQ_COMMON_SETTINGS_H
#define SOFTEQ_COMMON_SETTINGS_H

#include <map>
#include <memory>
#include <string>
#include <typeindex>

#include <softeq/common/any.hh>
#include <softeq/common/serialization/object_assembler.hh>

namespace softeq
{
namespace common
{
/*!
  \brief The list  of functions for settings manipulatation.
*/
class Settings
{
public:
    /**< details of serialization type */
    enum class SerializationType
    {
        Json,
        Xml
    };

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
            throw std::logic_error(stdutils::string_format("setting '%s' is not declared", name.c_str()));
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
                return any_cast<T>(entry.second.value);
            }
        }
        throw std::logic_error(
            stdutils::string_format("setting '%s' is not found", stdutils::demangle(typeid(T).name()).c_str()));
    }

    /*!
      Set default serialization parameters
      \param[in] path Path
      \param[in] type Serialization type
    */
    void setDefaultSerializationParameters(const std::string &path, SerializationType type);
    /*!
      Serialization
    */
    void serialize() const;
    /*!
      Serialization
       \param[in] path Path
       \param[in] type Serialization type
    */
    void serialize(const std::string &path, SerializationType type) const;
    /*!
      Deserialization
    */
    void deserialize();
    /*!
      Deserialization
      \param[in] path Path
    */
    void deserialize(const std::string &path);
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

        virtual void serialize(serialization::Serializer &serializer, const Any &node) const = 0;
        virtual void deserialize(serialization::Serializer &serializer, Any &node) = 0;
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

        virtual void serialize(serialization::Serializer &serializer, const Any &node) const override
        {
            try
            {
                const TYPE &value = any_cast<TYPE>(node);
                assemblerHolder.serialize(serializer, value);
            }
            catch (const std::bad_cast &e)
            {
                throw std::logic_error(stdutils::string_format("Expected '%s', but node contained %s at serialization",
                                                               stdutils::demangle(typeid(TYPE).name()).c_str(),
                                                               stdutils::demangle(node.type().name()).c_str()));
            }
        }

        virtual void deserialize(serialization::Serializer &serializer, Any &node) override
        {
            TYPE value;
            assemblerHolder.deserialize(serializer, value);
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
        Any value;
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
        _settings[name] = mapValue_t{Any(reference), std::move(ptr)};

        // try to deserialize a value from a file
        if (!_path.empty())
        {
            // copy value assemblers to stubSettings since mapValue_t has to have an assembler instance to get
            // deserialized
            std::map<std::string, mapValue_t> stubSettings{};
            stubSettings[name] = mapValue_t{Any(), _settings[name].assembler->clone()};
            deserialize(_path, stubSettings);

            if (stubSettings[name].value.hasValue())
            {
                if (stubSettings[name].value.type() == typeid(T))
                {
                    _settings[name].value = Any(stubSettings[name].value);
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

    void deserialize(const std::string &path, std::map<std::string, mapValue_t> &settings);

    std::map<std::string, mapValue_t> _settings;
    std::string _path;
    SerializationType _type;

public:
    using AccessKey = const int *;
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

template <typename T, softeq::common::Settings::AccessKey ACCESS_KEY>
unsigned int Settings::__macroOnlyRegistrator(const std::string &name)
{
    extern const int __settingsAccessKey;
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
    extern const int __settingsAccessKey;                                                                              \
    static unsigned int __prevent_placing_macro_within_functions_##TYPE()                                              \
    {                                                                                                                  \
        return Settings::__macroOnlyRegistrator<TYPE, &__settingsAccessKey>(NAME);                                     \
    }                                                                                                                  \
    static const unsigned int ___settings__##TYPE = __prevent_placing_macro_within_functions_##TYPE();

} // namespace common
} // namespace softeq

#endif // SOFTEQ_COMMON_SETTINGS_H
