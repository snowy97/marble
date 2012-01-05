//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010 Dennis Nienhüser <earthwings@gentoo.org>
// Copyright 2011 Thibaut Gridel <tgridel@free.fr>

#include "RunnerTask.h"

#include "MarbleAbstractRunner.h"
#include "MarbleDebug.h"
#include "routing/RouteRequest.h"

#include <QtCore/QTimer>

namespace Marble
{

RunnerTask::RunnerTask( MarbleAbstractRunner* runner ) :
        m_runner( runner )
{
    // nothing to do
}

void RunnerTask::run()
{
    runTask();

    runner()->deleteLater();
    emit finished( this );
}

MarbleAbstractRunner* RunnerTask::runner()
{
    return m_runner;
}

SearchTask::SearchTask(MarbleAbstractRunner* runner, const QString &searchTerm) :
        RunnerTask( runner ), m_searchTerm( searchTerm )
{
    // nothing to do
}

void SearchTask::runTask()
{
    runner()->search( m_searchTerm );
}

ReverseGeocodingTask::ReverseGeocodingTask( MarbleAbstractRunner* runner, const GeoDataCoordinates &coordinates ) :
        RunnerTask( runner ), m_coordinates( coordinates )
{
    // nothing to do
}

void ReverseGeocodingTask::runTask()
{
    runner()->reverseGeocoding( m_coordinates );
}

RoutingTask::RoutingTask( MarbleAbstractRunner* runner, const RouteRequest* routeRequest ) :
        RunnerTask( runner ), m_routeRequest( routeRequest )
{
    // nothing to do
}

void RoutingTask::runTask()
{
    runner()->retrieveRoute( m_routeRequest );
}

ParsingTask::ParsingTask( MarbleAbstractRunner* runner, const QString& fileName, DocumentRole role ) :
        RunnerTask( runner ), m_fileName( fileName ), m_role( role )
{
    // nothing to do
}

void ParsingTask::runTask()
{
    runner()->parseFile( m_fileName, m_role );
}

}

#include "RunnerTask.moc"
