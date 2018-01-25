#pragma once

#include "jcon.h"

class QString;

namespace jcon {

class JCON_API JsonRpcLogger
{
public:
    JsonRpcLogger();
    virtual ~JsonRpcLogger();

    virtual void logDebug(const QString& message);
    virtual void logInfo(const QString& message);
    virtual void logWarning(const QString& message);
    virtual void logError(const QString& message);
};

}
