//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010 Dennis Nienhüser <earthwings@gentoo.org>

#include "HostipRunner.h"

#include "MarbleAbstractRunner.h"
#include "MarbleDebug.h"
#include "GeoDataDocument.h"
#include "GeoDataPlacemark.h"

#include <QtCore/QMutex>
#include <QtCore/QString>
#include <QtCore/QTimer>
#include <QtCore/QVector>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

namespace Marble
{

HostipRunner::HostipRunner( QObject *parent ) :
        MarbleAbstractRunner( parent ),
        m_networkAccessManager( new QNetworkAccessManager( this ) )
{
}

HostipRunner::~HostipRunner()
{
}

GeoDataFeature::GeoDataVisualCategory HostipRunner::category() const
{
    return GeoDataFeature::Coordinate;
}

void HostipRunner::slotNoResults()
{
    emit searchFinished( QVector<GeoDataPlacemark*>() );
}

void HostipRunner::search( const QString &searchTerm )
{
    if( !searchTerm.contains('.') ) {
        // Simple IP/hostname heuristic to avoid requests not needed:
        // String must contain at least one dot.
        slotNoResults();
    }
    else {
        QVector<GeoDataPlacemark*> placemarks;

        QMutex mutex;
        mutex.lock();

        // Lookup the IP address for a hostname, or the hostname if an IP address was given
        QHostInfo ::lookupHost( searchTerm, this, SLOT(slotLookupFinished(QHostInfo)));

        m_waitCondition.wait( &mutex );

        if ( m_hostInfo.addresses().isEmpty() ) {
            emit searchFinished( placemarks );
            return;
        }

        QString hostAddress = m_hostInfo.addresses().first().toString();
        QString query = QString( "http://api.hostip.info/get_html.php?ip=%1&position=true" ).arg( hostAddress );
        m_request.setUrl( QUrl( query ) );

        // @todo FIXME Must currently be done in the main thread, see bug 257376
        QTimer::singleShot( 0, this, SLOT( get() ) );

        m_waitCondition.wait( &mutex );

        forever {
            const bool readyForRead = m_reply->waitForReadyRead( -1 );

            if ( m_reply->isFinished() || !readyForRead )
                break;
        }

        if ( m_reply->error() != QNetworkReply::NoError ) {
            mDebug() << Q_FUNC_INFO << "Network error:" << m_reply->errorString();
            emit searchFinished( placemarks );
            return;
        }

        double lon(0.0), lat(0.0);
        for ( QString line = m_reply->readLine(); !line.isEmpty(); line = m_reply->readLine() ) {
            QString lonInd = "Longitude: ";
            if ( line.startsWith(lonInd) ) {
                lon = line.mid( lonInd.length() ).toDouble();
            }

            QString latInd = "Latitude: ";
            if (line.startsWith( latInd) ) {
                lat = line.mid( latInd.length() ).toDouble();
            }
        }

        if (lon != 0.0 && lat != 0.0) {
            GeoDataPlacemark *placemark = new GeoDataPlacemark;

            placemark->setName( m_hostInfo.hostName() );

            QString description("%1 (%2)");
            placemark->setDescription( description.
                                     arg( m_hostInfo.hostName() ).
                                     arg( m_hostInfo.addresses().first().toString() ) );

            placemark->setCoordinate( lon * DEG2RAD, lat * DEG2RAD );
            placemark->setVisualCategory( category() );
            placemarks << placemark;
        }

        emit searchFinished( placemarks );
    }
}

void HostipRunner::slotLookupFinished(const QHostInfo &info)
{
    m_hostInfo = info;
    m_waitCondition.wakeAll();
}

void HostipRunner::get()
{
    m_reply = m_networkAccessManager->get( m_request );
    m_waitCondition.wakeAll();
}

} // namespace Marble

#include "HostipRunner.moc"
