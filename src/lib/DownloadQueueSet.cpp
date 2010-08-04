// Copyright 2009  Jens-Michael Hoffmann <jmho@c-xx.com>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either 
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public 
// License along with this library.  If not, see <http://www.gnu.org/licenses/>.

#include "DownloadQueueSet.h"

#include "MarbleDebug.h"

#include "HttpRequest.h"

namespace Marble
{

DownloadQueueSet::DownloadQueueSet( QObject * const parent )
    : QObject( parent )
{
}

DownloadQueueSet::DownloadQueueSet( DownloadPolicy const & policy, QObject * const parent )
    : QObject( parent ),
      m_downloadPolicy( policy )
{
}

DownloadQueueSet::~DownloadQueueSet()
{
    // todo: delete HttpRequests
}

DownloadPolicy DownloadQueueSet::downloadPolicy() const
{
    return m_downloadPolicy;
}

void DownloadQueueSet::setDownloadPolicy( DownloadPolicy const & policy )
{
    m_downloadPolicy = policy;
}

bool DownloadQueueSet::canAcceptRequest( const QUrl& sourceUrl,
                                     const QString& destinationFileName ) const
{
    if ( requestIsQueued( destinationFileName )) {
        mDebug() << "Download rejected: It's in the queue already:"
                 << destinationFileName;
        return false;
    }
    if ( requestIsWaitingForRetry( destinationFileName )) {
        mDebug() << "Download rejected: Will try to download again in some time:"
                 << destinationFileName;
        return false;
    }
    if ( requestIsActive( destinationFileName )) {
        mDebug() << "Download rejected: It's being downloaded already:"
                 << destinationFileName;
        return false;
    }
    if ( requestIsBlackListed( sourceUrl )) {
        mDebug() << "Download rejected: Blacklisted.";
        return false;
    }
    return true;
}

void DownloadQueueSet::addRequest( HttpRequest * const request )
{
    m_requests.push( request );
    mDebug() << "addRequest: new request queue size:" << m_requests.count();
    emit requestAdded();
    activateRequests();
}

void DownloadQueueSet::activateRequests()
{
    while ( !m_requests.isEmpty()
            && m_activeRequests.count() < m_downloadPolicy.maximumConnections() )
    {
        HttpRequest * const request = m_requests.pop();
        activateRequest( request );
    }
}

void DownloadQueueSet::retryRequests()
{
    while ( !m_retryQueue.isEmpty() ) {
        HttpRequest * const request = m_retryQueue.dequeue();
        mDebug() << "Requeuing" << request->destinationFileName();
        // FIXME: addRequest calls activateRequests every time
        addRequest( request );
    }
}

void DownloadQueueSet::finishRequest( HttpRequest * request, const QByteArray& data )
{
    mDebug() << "finishRequest: " << request->sourceUrl() << request->destinationFileName();

    deactivateRequest( request );
    emit requestRemoved();
    emit requestFinished( data, request->destinationFileName(), request->initiatorId() );
    request->deleteLater();
    activateRequests();
}

void DownloadQueueSet::redirectRequest( HttpRequest * request, const QUrl& newSourceUrl )
{
    mDebug() << "requestRedirected:" << request->sourceUrl() << " -> " << newSourceUrl;

    deactivateRequest( request );
    emit requestRemoved();
    emit requestRedirected( newSourceUrl, request->destinationFileName(), request->initiatorId(),
                        request->downloadUsage() );
    request->deleteLater();
}

void DownloadQueueSet::retryOrBlacklistRequest( HttpRequest * request, const int errorCode )
{
    Q_ASSERT( errorCode != 0 );
    Q_ASSERT( !m_retryQueue.contains( request ));

    deactivateRequest( request );
    emit requestRemoved();

    if ( request->tryAgain() ) {
        mDebug() << QString( "Download of %1 to %2 failed, but trying again soon" )
            .arg( request->sourceUrl().toString() ).arg( request->destinationFileName() );
        m_retryQueue.enqueue( request );
        emit requestRetry();
    }
    else {
        mDebug() << "JOB-address: " << request
                 << "Blacklist-size:" << m_requestBlackList.size()
                 << "err:" << errorCode;
        m_requestBlackList.insert( request->sourceUrl().toString() );
        mDebug() << QString( "Download of %1 Blacklisted. "
                             "Number of blacklist items: %2" )
            .arg( request->destinationFileName() )
            .arg( m_requestBlackList.size() );

        request->deleteLater();
    }
    activateRequests();
}

void DownloadQueueSet::activateRequest( HttpRequest * const request )
{
    m_activeRequests.push_back( request );

    connect( request, SIGNAL( requestDone( HttpRequest *, int )),
             SLOT( retryOrBlacklistRequest( HttpRequest *, int )));
    connect( request, SIGNAL( redirected( HttpRequest *, QUrl )),
             SLOT( redirectRequest( HttpRequest *, QUrl )));
    connect( request, SIGNAL( dataReceived( HttpRequest *, QByteArray )),
             SLOT( finishRequest( HttpRequest *, QByteArray )));

    request->execute();
}

/**
   pre condition: - request is in m_activeRequests
                  - request's signal are connected to our slots
   post condition: - request is not in m_activeRequests anymore (and btw not
                     in any other queue)
                   - request's signals are disconnected from our slots
 */
void DownloadQueueSet::deactivateRequest( HttpRequest * const request )
{
    const bool disconnected = request->disconnect();
    Q_ASSERT( disconnected );
    Q_UNUSED( disconnected ); // for Q_ASSERT in release mode
    const bool removed = m_activeRequests.removeOne( request );
    Q_ASSERT( removed );
    Q_UNUSED( removed ); // for Q_ASSERT in release mode
}

bool DownloadQueueSet::requestIsActive( QString const & destinationFileName ) const
{
    QList<HttpRequest*>::const_iterator pos = m_activeRequests.constBegin();
    QList<HttpRequest*>::const_iterator const end = m_activeRequests.constEnd();
    for (; pos != end; ++pos) {
        if ( (*pos)->destinationFileName() == destinationFileName ) {
            return true;
        }
    }
    return false;
}

inline bool DownloadQueueSet::requestIsQueued( QString const & destinationFileName ) const
{
    return m_requests.contains( destinationFileName );
}

bool DownloadQueueSet::requestIsWaitingForRetry( QString const & destinationFileName ) const
{
    QList<HttpRequest*>::const_iterator pos = m_retryQueue.constBegin();
    QList<HttpRequest*>::const_iterator const end = m_retryQueue.constEnd();
    for (; pos != end; ++pos) {
        if ( (*pos)->destinationFileName() == destinationFileName ) {
            return true;
        }
    }
    return false;
}

bool DownloadQueueSet::requestIsBlackListed( const QUrl& sourceUrl ) const
{
    QSet<QString>::const_iterator const pos =
        m_requestBlackList.constFind( sourceUrl.toString() );
    return pos != m_requestBlackList.constEnd();
}


inline bool DownloadQueueSet::RequestStack::contains( const QString& destinationFileName ) const
{
    return m_requestsContent.contains( destinationFileName );
}

inline int DownloadQueueSet::RequestStack::count() const
{
    return m_requests.count();
}

inline bool DownloadQueueSet::RequestStack::isEmpty() const
{
    return m_requests.isEmpty();
}

inline HttpRequest * DownloadQueueSet::RequestStack::pop()
{
    HttpRequest * const request = m_requests.pop();
    bool const removed = m_requestsContent.remove( request->destinationFileName() );
    Q_UNUSED( removed ); // for Q_ASSERT in release mode
    Q_ASSERT( removed );
    return request;
}

inline void DownloadQueueSet::RequestStack::push( HttpRequest * const request )
{
    m_requests.push( request );
    m_requestsContent.insert( request->destinationFileName() );
}


}

#include "DownloadQueueSet.moc"
