/* =================================================
 * This file is part of the TTK qmmp plugin project
 * Copyright (C) 2015 - 2021 Greedysky Studio

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License along
 * with this program; If not, see <http://www.gnu.org/licenses/>.
 ================================================= */

#ifndef XSFHELPER_H
#define XSFHELPER_H

#include <QMap>
#include <QFile>
#include <qmmp/qmmp.h>

class FileReader;

typedef struct {
    FileReader *reader;
    int bitrate;
    bool meta;
} xsf_info;

/*!
 * @author Greedysky <greedysky@163.com>
 */
class XSFHelper
{
public:
    explicit XSFHelper(const QString &path);
    ~XSFHelper();

    void deinit();
    void metaOnly(bool meta);

    bool initialize();
    int totalTime() const;
    void seek(qint64 time);

    int bitrate() const;
    int sampleRate() const;
    int channels() const;
    int bitsPerSample() const;

    int read(unsigned char *buf, int size);
    QMap<Qmmp::MetaData, QString> readMetaData() const;

private:
    QString m_path;
    xsf_info *m_info;

};

#endif
