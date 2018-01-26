
#ifndef PACK_P_H
#define PACK_P_H

#include "../msgpackcommon.h"

#include <QHash>
#include <QMetaType>
#include <QUuid>

class QByteArray;
class QString;

namespace MsgPackPrivate {
/* if wr (write) == false, packer just moves pointer forward
 *
 */

quint8 * pack(const QVariant &v, quint8 *p, bool wr, QVector<QByteArray> &user_data);

quint8 * pack_nil(quint8 *p, bool wr);

quint8 * pack_int(qint32 i, quint8 *p, bool wr);
quint8 * pack_uint(quint32 i, quint8 *p, bool wr);
quint8 * pack_longlong(qint64 i, quint8 *p, bool wr);
quint8 * pack_ulonglong(quint64 i, quint8 *p, bool wr);

quint8 * pack_bool(const QVariant &v, quint8 *p, bool wr);

quint8 * pack_arraylen(quint32 len, quint8 *p, bool wr);
quint8 * pack_array(const QVariantList &list, quint8 *p, bool wr, QVector<QByteArray> &user_data);
quint8 * pack_stringlist(const QStringList &list, quint8 *p, bool wr);

quint8 * pack_string_raw(const char *str, quint32 len, quint8 *p, bool wr);
quint8 * pack_string(const QString &str, quint8 *p, bool wr);
quint8 * pack_float(float f, quint8 *p, bool wr);
quint8 * pack_double(double i, quint8 *p, bool wr);
quint8 * pack_bin_header(quint32 len, quint8 *p, bool wr);
quint8 * pack_bin(const QByteArray &arr, quint8 *p, bool wr);
quint8 * pack_map(const QVariantMap &map, quint8 *p, bool wr, QVector<QByteArray> &user_data);
quint8 * pack_quuid(const QUuid &uuid, quint8 *p, bool wr);
}

#endif // PACK_P_H
