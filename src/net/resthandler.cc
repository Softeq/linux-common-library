#include <algorithm>
#include <memory>
#include <stdexcept>

#include <libxml/xmlwriter.h>

#include "softeq/common/net/resthandler.hh"
#include "softeq/common/stdutils.hh"
#include "softeq/common/fsutils.hh"
#include "softeq/common/log.hh"

using namespace softeq::common::net;

namespace
{
const char *const LOG_DOMAIN = "Rest";

class HelpCommand : public IDocCommand
{
public:
    HelpCommand(const std::string &name, const std::string &xslPath, const RestHandler::Commands &commands)
        : _commands(commands)
        , _name(name)
        , _xslPath(xslPath)
    {
    }

    std::string name() const override
    {
        return _name;
    }
    std::string description() const override
    {
        return "This command generates description about all added commands";
    }
    std::string requestDescription() const override
    {
        return "The command handles GET parameter 'xslt'. If it exists then it returns stylesheet.";
    }
    std::string responseDescription() const override
    {
        return "In case of 'xslt' param it returns text/xsl stylesheet. All other cases it returns text/xml commands "
               "descriptions";
    }

    bool perform(RestConnection &connection) override
    {
        if (connection.http().hasField("xslt"))
        {
            connection.http().setResponseHeader("Content-type", "text/xsl");
            if (softeq::common::fsutils::exist(_xslPath))
            {
                connection.http().sendFile(_xslPath);
            }
            else
            {
                connection.http() << R"(<?xml version="1.0" encoding="UTF-8"?>
                <xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:xs="http://www.w3.org/2001/XMLSchema" version="1.0" exclude-result-prefixes="xs">
                <xsl:output method="text"/>
                <xsl:template match="/commands">
                ERROR:Stylesheet is missing.
                Commands: <xsl:for-each select="command">
                *) <xsl:value-of select="name"/> - <xsl:value-of select="description"/>
                </xsl:for-each>
                </xsl:template>
                </xsl:stylesheet>
                )";
            }
        }
        else
        {
            connection.http().setResponseHeader("Content-type", "text/xml");
            describeCommands(connection.http());
        }
        return true;
    }

protected:
    /// Generate the commands description and put it into the stream.
    /// Watch out - it can throw an exception.

    void describeCommands(IHttpConnection &connection)
    {
        std::unique_ptr<xmlBuffer, std::function<void(xmlBuffer *)>> buf(xmlBufferCreate(), [](xmlBuffer *x) {
            if (x)
            {
                xmlBufferFree(x);
            }
        });
        if (!buf)
        {
            throw std::runtime_error("Error creating the XML buffer.");
        }
        std::unique_ptr<xmlTextWriter, std::function<void(xmlTextWriter *)>> writer(
            xmlNewTextWriterMemory(buf.get(), 0), [](xmlTextWriter *x) {
                if (x)
                {
                    xmlFreeTextWriter(x);
                }
            });
        if (!writer)
        {
            throw std::runtime_error("Error creating the XML writer.");
        }
        checkXmlError("document start", xmlTextWriterStartDocument(writer.get(), NULL, NULL, NULL));

        xmlTextWriterWriteFormatPI(writer.get(), BAD_CAST("xml-stylesheet"), "type=\"text/xsl\" href=\"?xslt\"");

        checkXmlError("root element start", xmlTextWriterStartElement(writer.get(), BAD_CAST "commands"));

        for (const IBaseCommand::UPtr &cmd : _commands)
        {
            xmlSerializeCommand(cmd.get(), writer.get());
        }

        checkXmlError("root element end", xmlTextWriterEndElement(writer.get()));
        checkXmlError("document end", xmlTextWriterEndDocument(writer.get()));

        connection << ((const char *)buf->content);
    }

private:
    const RestHandler::Commands &_commands;
    std::string _name;
    const std::string _xslPath;

    inline void checkXmlError(const std::string &descr, int code)
    {
        if (code < 0)
        {
            std::string error("Write ");
            error.append(descr);
            error.append(", error code=");
            error.append(std::to_string(code));
            throw std::runtime_error(error);
        }
    }

    inline void writeXmlElement(xmlTextWriterPtr writer, const std::string &elementName, const std::string &value)
    {
        const int res = xmlTextWriterWriteElement(writer, BAD_CAST elementName.c_str(), BAD_CAST value.c_str());
        std::string error("element ");
        error.append(elementName);
        checkXmlError(error, res);
    }

    void xmlSerializeCommand(const IBaseCommand *command, xmlTextWriterPtr writer)
    {
        checkXmlError("command element start", xmlTextWriterStartElement(writer, BAD_CAST "command"));

        writeXmlElement(writer, "name", command->name());
        const IDocCommand *docCommand = dynamic_cast<const IDocCommand *>(command);
        if (docCommand)
        {
            writeXmlElement(writer, "description", docCommand->description());
            writeXmlElement(writer, "request", docCommand->requestDescription());
            writeXmlElement(writer, "response", docCommand->responseDescription());
            writeXmlElement(writer, "requestGraph", docCommand->requestGraph());
            writeXmlElement(writer, "responseGraph", docCommand->responseGraph());
        }
        else
        {
            writeXmlElement(writer, "description", "The command is undocumented yet");
        }
        checkXmlError("command element end", xmlTextWriterEndElement(writer));
    }
};
} // namespace

RestConnection::RestConnection(IHttpConnection &connection)
    : _connection(connection)
{
}

IHttpConnection &RestConnection::http()
{
    return _connection;
}

void RestHandler::addCommand(IBaseCommand::UPtr &&command)
{
    _commands.push_back(std::move(command));
}

void RestHandler::removeCommand(const std::string &name)
{
    _commands.remove_if([&](IBaseCommand::UPtr &command) { return command->name() == name; });
}

bool RestHandler::handle(IHttpConnection &connection)
{
    const std::string path(connection.path());

    const std::string command_name(path.substr(1));
    for (const IBaseCommand::UPtr &command : _commands)
    {
        if (command->name() == command_name)
        {
            LOGD(LOG_DOMAIN, "Perform REST command: %s", command_name.c_str());
            RestConnection restConn(connection);

            try
            {
                if (!command->perform(restConn))
                {
                    throw std::logic_error("An error occurred at command execution.");
                }

                if (!connection.responseHasHeader("Content-type"))
                {
                    connection.setResponseHeader("Content-type", "application/json");
                }
                connection.setError(HttpStatusCode::STATUS_OK);
                LOGT(LOG_DOMAIN, "Successful execution of command %s.", command_name.c_str());
                return true;
            }
            catch (const std::exception &ex)
            {
                LOGE(LOG_DOMAIN, "Couldn't execute command %s. Reason: %s", command_name.c_str(), ex.what());
                if (!connection.error())
                {
                    connection.setError(HttpStatusCode::STATUS_INTERNAL_ERROR);
                    connection << "Internal server error: " << ex.what();
                }
                return false;
            }
        }
    }

    LOGE(LOG_DOMAIN, "Unknown command. URI: %s", connection.get().c_str());
    connection.setError(HttpStatusCode::STATUS_NOT_ALLOWED);
    return false;
}

RestAutodocHandler::RestAutodocHandler(const std::string &helpCommand, const std::string &xslPath)
{
    addCommand(IBaseCommand::UPtr(new HelpCommand(helpCommand, xslPath, _commands)));
}
