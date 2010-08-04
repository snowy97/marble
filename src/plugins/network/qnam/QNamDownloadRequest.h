//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009 Jens-Michael Hoffmann <jensmh@gmx.de>
//

#ifndef MARBLE_QNAM_DOWNLOAD_JOB_H
#define MARBLE_QNAM_DOWNLOAD_JOB_H

#include <QtNetwork/QNetworkReply>

#include "HttpRequest.h"


class QNetworkAccessManager;

namespace Marble
{

class QNamDownloadRequest: public HttpRequest
{
    Q_OBJECT

 public:
    QNamDownloadRequest( const QUrl & sourceUrl, const QString & destFileName,
                     const QString & id, QNetworkAccessManager * const );

    // HttpRequest abstract method
    virtual void execute();

 public Q_SLOTS:
    void downloadProgress( qint64 bytesReceived, qint64 bytesTotal );
    void error( QNetworkReply::NetworkError code );
    void finished();

 private:
    QNetworkAccessManager * m_networkAccessManager;
    QNetworkReply * m_networkReply;
};

}

#endif
