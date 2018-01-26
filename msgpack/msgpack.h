#ifndef MSGPACK_H
#define MSGPACK_H

#include "msgpack_export.h"
#include "msgpackcommon.h"

#include <QByteArray>
#include <QVariantList>

namespace MsgPack
{
    MSGPACK_EXPORT QVariant unpack(const QByteArray &data);
    MSGPACK_EXPORT QByteArray pack(const QVariant &variant);
} // MsgPack

#endif // MSGPACK_H
