//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2008 Henry de Valence <hdevalence@gmail.com>
// Copyright 2010 Dennis Nienhüser <earthwings@gentoo.org>
// Copyright 2011 Thibaut Gridel <tgridel@free.fr>

#include "MarbleRunnerManager.h"

#include "MarblePlacemarkModel.h"
#include "MarbleDebug.h"
#include "MarbleModel.h"
#include "Planet.h"
#include "GeoDataDocument.h"
#include "GeoDataPlacemark.h"
#include "PluginManager.h"
#include "RunnerPlugin.h"
#include "RunnerTask.h"
#include "routing/RouteRequest.h"
#include "routing/RoutingProfilesModel.h"

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtCore/QThreadPool>
#include <QtCore/QTimer>

namespace Marble
{

class MarbleModel;

class MarbleRunnerManager::Private
{
public:
    MarbleRunnerManager* q;
    MarbleModel * m_marbleModel;
    QVector<GeoDataDocument*> m_routingResult;

    Private( MarbleRunnerManager* parent, MarbleModel *model );

    ~Private();

    QList<RunnerTask*> m_routingTasks;

    void addRoutingResult( GeoDataDocument* route );

    void cleanupRoutingTask( RunnerTask* task );

};

MarbleRunnerManager::Private::Private( MarbleRunnerManager* parent, MarbleModel *model ) :
        q( parent ),
        m_marbleModel( model )
{
    qRegisterMetaType<GeoDataDocument*>( "GeoDataDocument*" );
    qRegisterMetaType<GeoDataPlacemark>( "GeoDataPlacemark" );
    qRegisterMetaType<GeoDataCoordinates>( "GeoDataCoordinates" );
    qRegisterMetaType<QVector<GeoDataPlacemark*> >( "QVector<GeoDataPlacemark*>" );
}

MarbleRunnerManager::Private::~Private()
{
    // nothing to do
}

QList<RunnerPlugin*> runnerPlugins( const MarbleModel *model, RunnerPlugin::Capability capability )
{
    const PluginManager *pluginManager = model->pluginManager();
    QList<RunnerPlugin*> plugins = pluginManager->runnerPlugins();

    QList<RunnerPlugin*> result;
    foreach( RunnerPlugin* plugin, plugins ) {
        if ( !plugin->supports( capability ) ) {
            continue;
        }

        if ( ( model && model->workOffline() && !plugin->canWorkOffline() ) ) {
            continue;
        }

        if ( !plugin->canWork( capability ) ) {
            continue;
        }

        if ( model && !plugin->supportsCelestialBody( model->planet()->id() ) )
        {
            continue;
        }

        result << plugin;
    }

    return result;
}

struct MarblePlacemarkSearch::Private
{
    Private( MarbleModel *model, MarblePlacemarkSearch *parent );

    void addSearchResult( QVector<GeoDataPlacemark*> result );
    void cleanupSearchTask( RunnerTask* task );

    MarblePlacemarkSearch *const q;
    MarbleModel *const m_marbleModel;
    QString m_lastSearchTerm;
    QMutex m_modelMutex;
    MarblePlacemarkModel *m_model;
    QVector<GeoDataPlacemark*> m_placemarkContainer;
    QList<RunnerTask*> m_searchTasks;
};

MarblePlacemarkSearch::Private::Private( MarbleModel *model, MarblePlacemarkSearch *parent ) :
    q( parent ),
    m_marbleModel( model ),
    m_model( new MarblePlacemarkModel( parent ) )
{
    m_model->setPlacemarkContainer( &m_placemarkContainer );
}

void MarblePlacemarkSearch::Private::cleanupSearchTask( RunnerTask* task )
{
    m_searchTasks.removeAll( task );

    if ( m_searchTasks.isEmpty() ) {
        emit q->searchFinished( m_lastSearchTerm );
    }
}

void MarbleRunnerManager::Private::cleanupRoutingTask( RunnerTask* task )
{
    m_routingTasks.removeAll( task );
    mDebug() << "removing task " << m_routingTasks.size() << " " << (long)task;
    if ( m_routingTasks.isEmpty() ) {
        if ( m_routingResult.isEmpty() ) {
            emit q->routeRetrieved( 0 );
        }

        emit q->routingFinished();
    }
}

MarbleRunnerManager::MarbleRunnerManager( MarbleModel *model, QObject *parent )
    : QObject( parent ), d( new Private( this, model ) )
{
    if ( QThreadPool::globalInstance()->maxThreadCount() < 4 ) {
        QThreadPool::globalInstance()->setMaxThreadCount( 4 );
    }
}

MarbleRunnerManager::~MarbleRunnerManager()
{
    delete d;
}

class MarbleReverseGeocoding::Private
{
public:
    Private( MarbleModel *model, MarbleReverseGeocoding *parent );

    void addReverseGeocodingResult( const GeoDataCoordinates &coordinates, const GeoDataPlacemark &placemark );

    MarbleReverseGeocoding *const q;
    MarbleModel *const m_marbleModel;
    QList<GeoDataCoordinates> m_reverseGeocodingResults;
    QString m_reverseGeocodingResult;
    QList<RunnerTask*> m_reverseTasks;
};

MarbleReverseGeocoding::Private::Private( MarbleModel *model, MarbleReverseGeocoding *parent ) :
    q( parent ),
    m_marbleModel( model )
{
}

MarbleReverseGeocoding::MarbleReverseGeocoding( MarbleModel *model, QObject *parent ) :
    QObject( parent ),
    d( new Private( model, this ) )
{
}

void MarbleReverseGeocoding::reverseGeocoding( const GeoDataCoordinates &coordinates )
{
    d->m_reverseGeocodingResults.removeAll( coordinates );
    QList<RunnerPlugin*> plugins = runnerPlugins( d->m_marbleModel, RunnerPlugin::ReverseGeocoding );
    foreach( RunnerPlugin* plugin, plugins ) {
        MarbleAbstractRunner* runner = plugin->newRunner();
        runner->setParent( this );
        connect( runner, SIGNAL( reverseGeocodingFinished( GeoDataCoordinates, GeoDataPlacemark ) ),
                 this, SLOT( addReverseGeocodingResult( GeoDataCoordinates, GeoDataPlacemark ) ) );
        runner->setModel( d->m_marbleModel );
        QThreadPool::globalInstance()->start( new ReverseGeocodingTask( runner, coordinates ) );
    }

    if ( plugins.isEmpty() ) {
        emit reverseGeocodingFinished( coordinates, GeoDataPlacemark() );
    }
}

MarblePlacemarkSearch::MarblePlacemarkSearch( MarbleModel *model, QObject *parent ) :
    QObject( parent ),
    d( new Private( model, this ) )
{
}

void MarblePlacemarkSearch::findPlacemarks( const QString &searchTerm )
{
    if ( searchTerm == d->m_lastSearchTerm ) {
      emit searchResultChanged( d->m_model );
      emit searchResultChanged( d->m_placemarkContainer );
      emit searchFinished( searchTerm );
      emit placemarkSearchFinished();
      return;
    }

    d->m_lastSearchTerm = searchTerm;

    d->m_searchTasks.clear();

    d->m_modelMutex.lock();
    d->m_model->removePlacemarks( "MarbleRunnerManager", 0, d->m_placemarkContainer.size() );
    qDeleteAll( d->m_placemarkContainer );
    d->m_placemarkContainer.clear();
    d->m_modelMutex.unlock();
    emit searchResultChanged( d->m_model );

    if ( searchTerm.trimmed().isEmpty() ) {
        emit searchFinished( searchTerm );
        emit placemarkSearchFinished();
        return;
    }

    QList<RunnerPlugin*> plugins = runnerPlugins( d->m_marbleModel, RunnerPlugin::Search );
    foreach( RunnerPlugin* plugin, plugins ) {
        MarbleAbstractRunner* runner = plugin->newRunner();
        runner->setParent( this );
        connect( runner, SIGNAL( searchFinished( QVector<GeoDataPlacemark*> ) ),
                 this, SLOT( addSearchResult( QVector<GeoDataPlacemark*> ) ) );
        runner->setModel( d->m_marbleModel );
        SearchTask* task = new SearchTask( runner, searchTerm );
        connect( task, SIGNAL( finished( RunnerTask* ) ), this, SLOT( cleanupSearchTask( RunnerTask* ) ) );
        d->m_searchTasks << task;
        mDebug() << "search task " << plugin->nameId() << " " << (long)task;
        QThreadPool::globalInstance()->start( task );
    }

    if ( plugins.isEmpty() ) {
        d->cleanupSearchTask( 0 );
    }
}

void MarblePlacemarkSearch::Private::addSearchResult( QVector<GeoDataPlacemark*> result )
{
    mDebug() << "Runner reports" << result.size() << " search results";
    if( result.isEmpty() )
        return;

    m_modelMutex.lock();
    int start = m_placemarkContainer.size();
    m_placemarkContainer << result;
    m_model->addPlacemarks( start, result.size() );
    m_modelMutex.unlock();
    emit q->searchResultChanged( m_model );
    emit q->searchResultChanged( m_placemarkContainer );
}

QVector<GeoDataPlacemark*> MarblePlacemarkSearch::searchPlacemarks( const QString &searchTerm ) {
    QEventLoop localEventLoop;
    QTimer watchdog;
    watchdog.setSingleShot(true);
    connect( &watchdog, SIGNAL(timeout()),
             &localEventLoop, SLOT(quit()));
    connect(this, SIGNAL(placemarkSearchFinished()),
            &localEventLoop, SLOT(quit()), Qt::QueuedConnection );

    watchdog.start( 30000 );
    findPlacemarks( searchTerm );
    localEventLoop.exec();
    return d->m_placemarkContainer;
}

void MarbleReverseGeocoding::Private::addReverseGeocodingResult( const GeoDataCoordinates &coordinates, const GeoDataPlacemark &placemark )
{
    if ( !m_reverseGeocodingResults.contains( coordinates ) && !placemark.address().isEmpty() ) {
        m_reverseGeocodingResults.push_back( coordinates );
        m_reverseGeocodingResult = placemark.address();
        emit q->reverseGeocodingFinished( coordinates, placemark );
    }

    if ( m_reverseTasks.isEmpty() ) {
        emit q->reverseGeocodingFinished();
    }
}

QString MarbleReverseGeocoding::searchReverseGeocoding( const GeoDataCoordinates &coordinates ) {
    QEventLoop localEventLoop;
    QTimer watchdog;
    watchdog.setSingleShot(true);
    connect( &watchdog, SIGNAL(timeout()),
             &localEventLoop, SLOT(quit()));
    connect(this, SIGNAL(reverseGeocodingFinished()),
            &localEventLoop, SLOT(quit()), Qt::QueuedConnection );

    watchdog.start( 30000 );
    reverseGeocoding( coordinates );
    localEventLoop.exec();
    return d->m_reverseGeocodingResult;
}

void MarbleRunnerManager::retrieveRoute( const RouteRequest *request )
{
    RoutingProfile profile = request->routingProfile();

    d->m_routingTasks.clear();
    d->m_routingResult.clear();

    QList<RunnerPlugin*> plugins = runnerPlugins( d->m_marbleModel, RunnerPlugin::Routing );
    foreach( RunnerPlugin* plugin, plugins ) {
        if ( !profile.name().isEmpty() && !profile.pluginSettings().contains( plugin->nameId() ) ) {
            continue;
        }

        MarbleAbstractRunner* runner = plugin->newRunner();
        runner->setParent( this );
        connect( runner, SIGNAL( routeCalculated( GeoDataDocument* ) ),
                 this, SLOT( addRoutingResult( GeoDataDocument* ) ) );
        runner->setModel( d->m_marbleModel );
        RoutingTask* task = new RoutingTask( runner, request );
        connect( task, SIGNAL( finished( RunnerTask* ) ), this, SLOT( cleanupRoutingTask( RunnerTask* ) ) );
        mDebug() << "route task " << plugin->nameId() << " " << (long)task;
        d->m_routingTasks << task;
        QThreadPool::globalInstance()->start( task );
    }

    if ( plugins.isEmpty() ) {
        mDebug() << "No routing plugins found, cannot retrieve a route";
        d->cleanupRoutingTask( 0 );
    }
}

void MarbleRunnerManager::Private::addRoutingResult( GeoDataDocument* route )
{
    if ( route ) {
        mDebug() << "route retrieved";
        m_routingResult.push_back( route );
        emit q->routeRetrieved( route );
    }
}

QVector<GeoDataDocument*> MarbleRunnerManager::searchRoute( const RouteRequest *request ) {
    QEventLoop localEventLoop;
    QTimer watchdog;
    watchdog.setSingleShot(true);
    connect( &watchdog, SIGNAL(timeout()),
             &localEventLoop, SLOT(quit()));
    connect(this, SIGNAL(routingFinished()),
            &localEventLoop, SLOT(quit()), Qt::QueuedConnection );

    watchdog.start( 30000 );
    retrieveRoute( request );
    localEventLoop.exec();
    return d->m_routingResult;
}

class MarbleFileParser::Private
{
public:
    Private( MarbleModel *model, MarbleFileParser *parent );

    void addParsingResult( GeoDataDocument* document, const QString& error = QString() );
    void cleanupParsingTask( RunnerTask* task );

    MarbleFileParser *const q;
    MarbleModel *const m_marbleModel;
    GeoDataDocument* m_fileResult;
    QList<RunnerTask*> m_parsingTasks;
};

MarbleFileParser::Private::Private( MarbleModel *model, MarbleFileParser *parent ) :
    q( parent ),
    m_marbleModel( model ),
    m_fileResult( 0 )
{
}

void MarbleFileParser::Private::cleanupParsingTask( RunnerTask* task )
{
    m_parsingTasks.removeAll( task );
    mDebug() << "removing task " << m_parsingTasks.size() << " " << (long)task;

    if ( m_parsingTasks.isEmpty() ) {
        emit q->parsingFinished();
    }
}

MarbleFileParser::MarbleFileParser( MarbleModel *model, QObject *parent ) :
    QObject( parent ),
    d( new Private( model, this ) )
{
}

void MarbleFileParser::parseFile( const QString &fileName, DocumentRole role )
{
    QList<RunnerPlugin*> plugins = runnerPlugins( d->m_marbleModel, RunnerPlugin::Parsing );
    foreach( RunnerPlugin *plugin, plugins ) {
        MarbleAbstractRunner* runner = plugin->newRunner();
        connect( runner, SIGNAL( parsingFinished( GeoDataDocument*, QString ) ),
                 this, SLOT( addParsingResult( GeoDataDocument*, QString )) );
        ParsingTask *task = new ParsingTask( runner, fileName, role );
        connect( task, SIGNAL( finished( RunnerTask* ) ), this, SLOT( cleanupParsingTask(RunnerTask*) ) );
        mDebug() << "parse task " << plugin->nameId() << " " << (long)task;
        d->m_parsingTasks << task;
        QThreadPool::globalInstance()->start( task );
    }

    if ( plugins.isEmpty() ) {
        emit parsingFinished( 0, "No plugin found");
        d->cleanupParsingTask( 0 );
    }
}

void MarbleFileParser::Private::addParsingResult( GeoDataDocument *document, const QString& error )
{
    if ( document || !error.isEmpty() ) {
        m_fileResult = document;
        emit q->parsingFinished( document, error );
    }
}

GeoDataDocument* MarbleFileParser::openFile( const QString &fileName, DocumentRole role ) {
    QEventLoop localEventLoop;
    QTimer watchdog;
    watchdog.setSingleShot(true);
    connect( &watchdog, SIGNAL(timeout()),
             &localEventLoop, SLOT(quit()));
    connect(this, SIGNAL(parsingFinished()),
            &localEventLoop, SLOT(quit()), Qt::QueuedConnection );

    watchdog.start( 30000 );
    parseFile( fileName, role);
    localEventLoop.exec();
    return d->m_fileResult;
}

}

#include "MarbleRunnerManager.moc"
