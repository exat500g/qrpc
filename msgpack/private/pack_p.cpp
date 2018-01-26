// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "pack_p.h"
#include "../endianhelper.h"

#include <limits>

#include <QByteArray>
#include <QDebug>
#include <QMapIterator>
#include <QString>
#include <QStringList>
#include <QUuid>

static const bool compatibilityMode=false;

quint8 *MsgPackPrivate::pack(const QVariant &v, quint8 *p, bool wr, QVector<QByteArray> &user_data)
{
    QMetaType::Type t = (QMetaType::Type)v.type();
    if (v.isNull() && !v.isValid())
        p = pack_nil(p, wr);
    else if (t == QMetaType::Int)
        p = pack_int(v.toInt(), p, wr);
    else if (t == QMetaType::UInt)
        p = pack_uint(v.toUInt(), p, wr);
    else if (t == QMetaType::Bool)
        p = pack_bool(v, p, wr);
    else if (t == QMetaType::QString)
        p = pack_string(v.toString(), p, wr);
    else if (t == QMetaType::QVariantList)
        p = pack_array(v.toList(), p, wr, user_data);
    else if (t == QMetaType::QStringList)
        p = pack_stringlist(v.toStringList(), p, wr);
    else if (t == QMetaType::LongLong)
        p = pack_longlong(v.toLongLong(), p, wr);
    else if (t == QMetaType::ULongLong)
        p = pack_ulonglong(v.toULongLong(), p, wr);
    else if (t == QMetaType::Double)
        p = pack_double(v.toDouble(), p, wr);
    else if (t == QMetaType::Float)
        p = pack_float(v.toFloat(), p, wr);
    else if (t == QMetaType::QByteArray)
        p = pack_bin(v.toByteArray(), p, wr);
    else if (t == QMetaType::QVariantMap)
        p = pack_map(v.toMap(), p, wr, user_data);
    else if (t == QMetaType::QUuid)
        p = pack_quuid(v.toUuid(), p, wr);
    else {
        qCritical() << "MsgPack::pack can't pack type:" << t;
    }

    return p;
}

quint8 *MsgPackPrivate::pack_nil(quint8 *p, bool wr)
{
    if (wr) *p = MsgPack::FirstByte::NIL;
    return p + 1;
}

quint8 *MsgPackPrivate::pack_int(qint32 i, quint8 *p, bool wr)
{
    if (i >= -32 && i <= 127) {
        qint32 val = _msgpack_be32(i);
        if (wr) *p = *( (quint8 *)&val + 3 );
        p++;
    } else if (i >= std::numeric_limits<qint8>::min()
               && i <= std::numeric_limits<quint8>::max()) {
        if (wr) *p = i > 0 ? 0xcc : 0xd0;
        p++;
        if (wr) *p = i;
        p++;
    } else if (i >= std::numeric_limits<qint16>::min() &&
               i <= std::numeric_limits<quint16>::max()) {
        if (wr) *p = i > 0 ? 0xcd : 0xd1;
        p++;
        if (wr) _msgpack_store16(p, i);
        p += 2;
    } else {
        if (wr) *p = i > 0 ? 0xce : 0xd2;
        p++;
        if (wr) _msgpack_store32(p, i);
        p += 4;
    }

    return p;
}

quint8 *MsgPackPrivate::pack_uint(quint32 i, quint8 *p, bool wr)
{
    if (i <= 127) {
        qint32 val = _msgpack_be32(i);
        if (wr) *p = *( (quint8 *)&val + 3 );
        p++;
    } else if (i <= std::numeric_limits<quint8>::max()) {
        if (wr) *p = 0xcc;
        p++;
        if (wr) *p = i;
        p++;
    } else if (i <= std::numeric_limits<quint16>::max()) {
        if (wr) *p = 0xcd;
        p++;
        if (wr) _msgpack_store16(p, i);
        p += 2;
    } else {
        if (wr) *p = 0xce;
        p++;
        if (wr) _msgpack_store32(p, i);
        p += 4;
    }

    return p;
}

quint8 *MsgPackPrivate::pack_longlong(qint64 i, quint8 *p, bool wr)
{
    if (i >= 0 && i <= std::numeric_limits<quint32>::max())
        return pack_uint(i, p, wr);
    else if (i >= std::numeric_limits<qint32>::min()
            && i <= std::numeric_limits<qint32>::max())
        return pack_int(i, p, wr);
    if (wr) *p = 0xd3;
    p++;
    if (wr) _msgpack_store64(p, i);
    return p + 8;
}

quint8 *MsgPackPrivate::pack_ulonglong(quint64 i, quint8 *p, bool wr)
{
    if (i <= std::numeric_limits<quint32>::max())
        return pack_uint(i, p, wr);
    if (wr) *p = 0xcf;
    p++;
    if (wr) _msgpack_store64(p, i);
    return p + 8;
}

quint8 *MsgPackPrivate::pack_bool(const QVariant &v, quint8 *p, bool wr)
{
    if (wr)
        *p = v.toBool() ? 0xc3 : 0xc2;
    return p + 1;
}

quint8 *MsgPackPrivate::pack_arraylen(quint32 len, quint8 *p, bool wr)
{
    if (len <= 15) {
        if (wr) *p = 0x90 | len;
        p++;
    } else if (len <= std::numeric_limits<quint16>::max()) {
        if (wr) *p = 0xdc;
        p++;
        if (wr) _msgpack_store16(p, len);
        p += 2;
    } else {
        if (wr) *p = 0xdd;
        p++;
        if (wr) _msgpack_store32(p, len);
        p += 4;
    }
    return p;
}

quint8 *MsgPackPrivate::pack_array(const QVariantList &list, quint8 *p, bool wr, QVector<QByteArray> &user_data)
{
    int len = list.length();
    p = pack_arraylen(len, p, wr);
    foreach (QVariant item, list)
        p = pack(item, p, wr, user_data);
    return p;
}

quint8 *MsgPackPrivate::pack_stringlist(const QStringList &list, quint8 *p, bool wr)
{
    int len = list.length();
    p = pack_arraylen(len, p, wr);
    foreach (QString item, list)
        p = pack_string(item, p, wr);
    return p;
}

quint8 *MsgPackPrivate::pack_string_raw(const char *str, quint32 len, quint8 *p, bool wr)
{
    if (len <= 31) {
        if (wr) *p = 0xa0 | len;
        p++;
    } else if (len <= std::numeric_limits<quint8>::max() &&
               compatibilityMode == false) {
        if (wr) *p = 0xd9;
        p++;
        if (wr) *p = len;
        p++;
    } else if (len <= std::numeric_limits<quint16>::max()) {
        if (wr) *p = 0xda;
        p++;
        if (wr) _msgpack_store16(p, len);
        p += 2;
    } else {
        if (wr) *p = 0xdb;
        p++;
        if (wr) _msgpack_store32(p, len);
        p += 4;
    }
    if (wr) memcpy(p, str, len);

    return p + len;
}

quint8 *MsgPackPrivate::pack_string(const QString &str, quint8 *p, bool wr)
{
    QByteArray str_data = str.toUtf8();
    quint32 str_len = str_data.length();
    return pack_string_raw(str_data.data(), str_len, p, wr);
}

quint8 *MsgPackPrivate::pack_float(float f, quint8 *p, bool wr)
{
    if (wr) *p = 0xca;
    p++;
    if (wr) {
        quint8 *d = (quint8 *)&f;
#ifdef __LITTLE_ENDIAN__
        for (int i = 0; i < 4; ++i)
            *(p + 3 - i) = *(d + i);
#else
        for (int i = 0; i < 4; ++i)
            *(p + i) = *(d + i);
#endif
    }
    return p + 4;
}

quint8 *MsgPackPrivate::pack_double(double i, quint8 *p, bool wr)
{
    if (wr) *p = 0xcb;
    p++;
    if (wr) {
        quint8 *d = (quint8 *)&i;
#ifdef __LITTLE_ENDIAN__
        for (int i = 0; i < 8; ++i)
            *(p + 7 - i) = *(d + i);
#else
        for (int i = 0; i < 8; ++i)
            *(p + i) = *(d + i);
#endif
    }
    return p + 8;
}

quint8 *MsgPackPrivate::pack_bin_header(quint32 len, quint8 *p, bool wr)
{
    if (len <= std::numeric_limits<quint8>::max()) {
        if (wr) *p = compatibilityMode ? 0xd9 : 0xc4;
        p++;
        if (wr) *p = len;
        p++;
    } else if (len <= std::numeric_limits<quint16>::max()) {
        if (wr) *p = compatibilityMode ? 0xda : 0xc5;
        p++;
        if (wr) _msgpack_store16(p, len);
        p += 2;
    } else {
        if (wr) *p = compatibilityMode ? 0xdb : 0xc6;
        p++;
        if (wr) _msgpack_store32(p, len);
        p += 4;
    }
    return p;
}

quint8 *MsgPackPrivate::pack_bin(const QByteArray &arr, quint8 *p, bool wr)
{
    quint32 len = arr.length();
    p = pack_bin_header(len, p, wr);
    if (wr) memcpy(p, arr.data(), len);
    p += len;
    return p;
}


quint8 *MsgPackPrivate::pack_map(const QVariantMap &map, quint8 *p, bool wr, QVector<QByteArray> &user_data)
{
    QMapIterator<QString, QVariant> it(map);
    int len = 0;
    while (it.hasNext()) {
        it.next();
        len++;
    }
    if (len <= 15) {
        if (wr) *p = 0x80 | len;
        p++;
    } else if (len <= std::numeric_limits<quint16>::max()) {
        if (wr) *p = 0xde;
        p++;
        if (wr) _msgpack_store16(p, len);
        p += 2;
    } else {
        if (wr) *p = 0xdf;
        p++;
        if (wr) _msgpack_store32(p, len);
        p += 4;
    }

    it.toFront();
    while (it.hasNext()) {
        it.next();
        p = pack(it.key(), p, wr, user_data);
        p = pack(it.value(), p, wr, user_data);
    }
    return p;
}

quint8 *MsgPackPrivate::pack_quuid(const QUuid& uuid, quint8 *p, bool wr){
    if (wr){
        p[0] = 0xd8; //FIXEX16
        p[1] = QMetaType::QUuid;
    }
    p+=2;
    Q_ASSERT(uuid.toRfc4122().size()==16);
    if (wr) memcpy(p, uuid.toRfc4122().data(), 16);
    p += 16;
    return p;
}
