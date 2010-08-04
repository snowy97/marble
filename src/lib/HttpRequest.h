//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2007      Torsten Rahn <tackat@kde.org>
// Copyright 2007      Inge Wallin  <ingwa@kde.org>
// Copyright 2008,2009 Jens-Michael Hoffmann <jensmh@gmx.de>
// Copyright 2008      Pino Toscano <pino@kde.org>
//

#ifndef MARBLE_HTTPREQUEST_H
#define MARBLE_HTTPREQUEST_H

#include <QtCore/QByteArray>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QUrl>

#include "global.h"

#include "marble_export.h"

namespace Marble
{
class HttpRequestPrivate;

class MARBLE_EXPORT HttpRequest: public QObject
{
    Q_OBJECT

 public:
    HttpRequest( const QUrl & sourceUrl, const QString & destFileName, const QString &id );
    ~HttpRequest();

    QUrl sourceUrl() const;
    void setSourceUrl( const QUrl & );

    QString initiatorId() const;
    void setInitiatorId( const QString & );

    QString destinationFileName() const;
    void setDestinationFileName( const QString & );

    bool tryAgain();

    DownloadUsage downloadUsage() const;
    void setDownloadUsage( const DownloadUsage );

    void setUserAgentPluginId( const QString & pluginId ) const;

    QByteArray userAgent() const;

 Q_SIGNALS:
    /**
     * errorCode contains 0, if there was no error and 1 otherwise
     */
    void requestDone( HttpRequest *, int errorCode );
    void redirected( HttpRequest * request, QUrl redirectionTarget );

    /**
     * This signal is emitted if the data was successfully received and
     * the argument data contains completely the downloaded content.
     */
    void dataReceived( HttpRequest * request, QByteArray data );

 public Q_SLOTS:
    virtual void execute() = 0;

 private:
    Q_DISABLE_COPY( HttpRequest )
    HttpRequestPrivate *const d;
    friend class HttpRequestPrivate;
};

}

#endif
