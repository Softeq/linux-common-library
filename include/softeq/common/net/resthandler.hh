#ifndef SOFTEQ_COMMON_RESTHANDLER_H_
#define SOFTEQ_COMMON_RESTHANDLER_H_

/*!
  \file
  \brief Header file, in which declared classes and interfaces needed to handling requests via REST API.
*/

#include <list>
#include <functional>
#include <iostream>
#include <string>
#include <type_traits>

#include "softeq/common/optional.hh"
#include "softeq/common/stdutils.hh"
#include "http_server.hh"
#include "http_connection.hh"

#include "softeq/common/serialization/json_helpers.hh"

namespace softeq
{
namespace common
{
namespace net
{
/*!
  \brief Class, intended to hide internal representation of data (JSON) transferred via HTTP.

  In fact it is class adapter to instance of HttpConnection, which provides interface to
  JSON encoding/decoding of entities, specific for REST API.
*/
class RestConnection
{
    IHttpConnection &_connection; /// adaptee connection

public:
    explicit RestConnection(IHttpConnection &connection);

    template <typename T>
    T input()
    {
        return softeq::common::serialization::deserializeObjectFromJson<T>(_connection.body());
    };

    template <typename T>
    void setOutput(T &object)
    {
        _connection << softeq::common::serialization::serializeObjectToJson(object);
    };

    IHttpConnection &http();
};

/*!
  \brief Base class, which declares interface to command of REST API.
  Each command in REST API must implement this interface to be handled by server.
  Command is identified by descriptive name and determines behavior specific for its purpose.
*/
class IBaseCommand
{
public:
    virtual ~IBaseCommand() = default;

    /// The method running the command.
    virtual bool perform(RestConnection &connection) = 0;

    /// Get the command name.
    virtual std::string name() const = 0;
    using UPtr = std::unique_ptr<IBaseCommand>;
};

/// A web command should usually inherit from a descendant of this class.
//   template<typename Request_t, typename Response_t>
class IDocCommand : public IBaseCommand
{
    std::string _requestGraph;
    std::string _responseGraph;

public:
    template <typename T>
    void defineRequest()
    {
        _requestGraph = softeq::common::serialization::ObjectAssembler<T>::accessor().graph();
    }

    template <typename T>
    void defineResponse()
    {
        _responseGraph = softeq::common::serialization::ObjectAssembler<T>::accessor().graph();
    }

    /// These methods should return the human readable:
    /// command description, input(request) and output(response).
    virtual std::string description() const = 0;
    virtual std::string requestDescription() const = 0;
    virtual std::string responseDescription() const = 0;
    std::string requestGraph() const
    {
        return _requestGraph;
    }

    std::string responseGraph() const
    {
        return _responseGraph;
    }
};

/*!
  \brief Class of HTTP-server, intended to handle REST API commands.

  This class implements HTTP-server, which accept request at the specified port,
  handles them, using installed commands and replies to client.
*/
class RestHandler : public IHttpConnectionDispatcher
{
public:
    using Commands = std::list<IBaseCommand::UPtr>;

protected:
    Commands _commands;

public:
    void addCommand(IBaseCommand::UPtr &&command);
    void removeCommand(const std::string &name);

    bool handle(IHttpConnection &connection) override;

    /// Run a function for each command, added to the internal command list.
    template <typename return_t, typename... arguments_t>
    void forEach(std::function<return_t(IBaseCommand &, arguments_t...)> &func, arguments_t... args)
    {
        for (IBaseCommand::UPtr &cmd : _commands)
        {
            if (cmd)
                func(*cmd, args...);
        }
    }
};

class RestAutodocHandler : public RestHandler
{
public:
    RestAutodocHandler(const std::string &helpCommand, const std::string &xslPath);
};

} // namespace net
} // namespace common
} // namespace softeq

#endif // SOFTEQ_COMMON_RESTHANDLER_H_
