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

#ifndef MARBLE_DOWNLOADQUEUESET_H
#define MARBLE_DOWNLOADQUEUESET_H

#include <QtCore/QList>
#include <QtCore/QQueue>
#include <QtCore/QObject>
#include <QtCore/QSet>
#include <QtCore/QStack>
#include <QtCore/QUrl>

#include "DownloadPolicy.h"

namespace Marble
{

class HttpRequest;

/**
   Life of a HttpRequest
   =================
   - Request is added to the QueueSet (by calling addRequest() )
     the HttpRequest is put into the m_requestQueue where it waits for "activation"
     signal requestAdded is emitted
   - Request is activated
     Request is moved from m_requestQueue to m_activeRequests and signals of the request
     are connected to slots (local or HttpDownloadManager)
     Request is executed by calling the requests execute() method

   now there are different possibilities:
   1) Request emits requestDone (some error occurred, or canceled (kio))
      Request is disconnected
      signal requestRemoved is emitted
      Request is either moved from m_activeRequests to m_retryQueue
        or destroyed and blacklisted

   2) Request emits redirected
      Request is removed from m_activeRequests, disconnected and destroyed
      signal requestRemoved is emitted
      (HttpDownloadManager creates new Request with new source url)

   3) Request emits dataReceived
      Request is removed from m_activeRequests, disconnected and destroyed
      signal requestRemoved is emitted

   so we can conclude following rules:
   - Request is only connected to signals when in "active" state


   questions:
   - update of initiatorId needed?
     "We update the initiatorId as the previous initiator
      likely doesn't exist anymore"
   - blacklist or black list?

 */

class DownloadQueueSet: public QObject
{
    Q_OBJECT

 public:
    explicit DownloadQueueSet( QObject * const parent = 0 );
    explicit DownloadQueueSet( const DownloadPolicy& policy, QObject * const parent = 0 );
    ~DownloadQueueSet();

    DownloadPolicy downloadPolicy() const;
    void setDownloadPolicy( const DownloadPolicy& );

    bool canAcceptRequest( const QUrl& sourceUrl,
                       const QString& destinationFileName ) const;
    void addRequest( HttpRequest * const request );

    void activateRequests();
    void retryRequests();

 Q_SIGNALS:
    void requestAdded();
    void requestRemoved();
    void requestRetry();
    void requestFinished( const QByteArray& data, const QString& destinationFileName,
                      const QString& id );
    void requestRedirected( const QUrl& newSourceUrl, const QString& destinationFileName,
                        const QString& id, DownloadUsage );

 private Q_SLOTS:
    void finishRequest( HttpRequest * request, const QByteArray& data );
    void redirectRequest( HttpRequest * request, const QUrl& newSourceUrl );
    void retryOrBlacklistRequest( HttpRequest * request, const int errorCode );

 private:
    void activateRequest( HttpRequest * const request );
    void deactivateRequest( HttpRequest * const request );
    bool requestIsActive( const QString& destinationFileName ) const;
    bool requestIsQueued( const QString& destinationFileName ) const;
    bool requestIsWaitingForRetry( const QString& destinationFileName ) const;
    bool requestIsBlackListed( const QUrl& sourceUrl ) const;

    DownloadPolicy m_downloadPolicy;

    /** This is the first stage a request enters, from this queue it will get
     *  into the activatedRequests container.
     */
    class RequestStack
    {
    public:
        bool contains( const QString& destinationFileName ) const;
        int count() const;
        bool isEmpty() const;
        HttpRequest * pop();
        void push( HttpRequest * const );
    private:
        QStack<HttpRequest*> m_requests;
        QSet<QString> m_requestsContent;
    };
    RequestStack m_requests;

    /// Contains the requests which are currently being downloaded.
    QList<HttpRequest*> m_activeRequests;

    /** Contains requests which failed to download and which are scheduled for
     *  retry according to retry settings.
     */
    QQueue<HttpRequest*> m_retryQueue;

    /// Contains the blacklisted source urls
    QSet<QString> m_requestBlackList;
};

}

#endif
