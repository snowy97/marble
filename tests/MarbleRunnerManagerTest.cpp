//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2011 Thibaut Gridel <tgridel@free.fr>
// Copyright 2012 Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#include <QtTest/QtTest>
#include <QtTest/QSignalSpy>
#include <QtCore/QMetaType>

#include "MarbleDebug.h"
#include "MarbleDirs.h"
#include "MarbleModel.h"
#include "MarbleRunnerManager.h"
#include "GeoDataPlacemark.h"
#include "routing/RouteRequest.h"

#define addRow() QTest::newRow( QString("line %1").arg( __LINE__ ).toAscii().data() )

Q_DECLARE_METATYPE( QList<Marble::GeoDataCoordinates> )

namespace Marble
{

class MarbleRunnerManagerTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.

    void findPlacemarks_data();
    void findPlacemarks();

    void reverseGeocoding();

    void retrieveRoute_data();
    void retrieveRoute();

    void parseFile_data();
    void parseFile();

public:
    MarbleModel *m_model;
    GeoDataCoordinates m_coords;
    GeoDataCoordinates m_coords2;
};

void MarbleRunnerManagerTest::initTestCase()
{
    // MarbleDebug::enable = true;
    MarbleDirs::setMarbleDataPath( DATA_PATH );
    MarbleDirs::setMarblePluginPath( PLUGIN_PATH );

    qRegisterMetaType<QList<GeoDataCoordinates> >( "QList<GeoDataCoordinates>" );

    m_coords.setLatitude(52.50160, GeoDataCoordinates::Degree );
    m_coords.setLongitude(13.40233, GeoDataCoordinates::Degree );

    m_coords2.setLatitude(52.52665, GeoDataCoordinates::Degree );
    m_coords2.setLongitude(13.39032, GeoDataCoordinates::Degree );

    m_model = new MarbleModel( this );
    m_model->setWorkOffline( false );
}

void MarbleRunnerManagerTest::cleanupTestCase()
{
    delete m_model;
    m_model = 0;
}

void MarbleRunnerManagerTest::findPlacemarks_data()
{
    QTest::addColumn<QString>( "name" );

    addRow() << QString( "Berlin" );
    addRow() << QString( "www.google.com" );
}

void MarbleRunnerManagerTest::findPlacemarks()
{
    MarblePlacemarkSearch m_runnerManager( m_model, this );

    QSignalSpy finishSpy( &m_runnerManager, SIGNAL( searchFinished( QString ) ) );
    QSignalSpy resultSpy( &m_runnerManager, SIGNAL( searchResultChanged( QVector<GeoDataPlacemark*> ) ) );

    QFETCH( QString, name );
    m_runnerManager.findPlacemarks( name );

    while ( finishSpy.count() == 0 ) {
        qApp->processEvents();
    }

    QCOMPARE( resultSpy.count(), 1 );
    QCOMPARE( finishSpy.count(), 1 );
}

void MarbleRunnerManagerTest::reverseGeocoding()
{
    MarbleReverseGeocoding m_runnerManager(m_model, this);

    QSignalSpy finishSpy( &m_runnerManager, SIGNAL(reverseGeocodingFinished()) );
    QSignalSpy resultSpy( &m_runnerManager, SIGNAL(reverseGeocodingFinished(GeoDataCoordinates,GeoDataPlacemark)) );

    m_runnerManager.reverseGeocoding( m_coords );

    while ( finishSpy.count() == 0 ) {
        qApp->processEvents();
    }

    QCOMPARE( resultSpy.count(), 1 );
    QCOMPARE( finishSpy.count(), 1 );
}

void MarbleRunnerManagerTest::retrieveRoute_data()
{
    QTest::addColumn<QList<GeoDataCoordinates> >( "coordinatesList" );

    addRow() << ( QList<GeoDataCoordinates>() << m_coords << m_coords2 );
}

void MarbleRunnerManagerTest::retrieveRoute()
{
    MarbleRunnerManager m_runnerManager(m_model, this);

    QSignalSpy finishSpy( &m_runnerManager, SIGNAL(routingFinished()) );
    QSignalSpy resultSpy( &m_runnerManager, SIGNAL( routeRetrieved(GeoDataDocument*)) );

    QFETCH( QList<GeoDataCoordinates>, coordinatesList );
    RouteRequest request;
    foreach( const GeoDataCoordinates &coordinates, coordinatesList ) {
        request.append( coordinates );
    }

    m_runnerManager.retrieveRoute( &request );

    while ( finishSpy.count() == 0 ) {
        qApp->processEvents();
    }

    QVERIFY( resultSpy.count() > 0 );
    QCOMPARE( finishSpy.count(), 1 );
}

void MarbleRunnerManagerTest::parseFile_data()
{
    QTest::addColumn<QString>( "fileName" );

    addRow() << MarbleDirs::path( "placemarks/otherplacemarks.cache" );
}

void MarbleRunnerManagerTest::parseFile()
{
    MarbleFileParser m_runnerManager(m_model, this);

    QSignalSpy finishSpy( &m_runnerManager, SIGNAL( parsingFinished() ) );
    QSignalSpy resultSpy( &m_runnerManager, SIGNAL( parsingFinished(GeoDataDocument*,QString)) );

    QFETCH( QString, fileName );

    m_runnerManager.parseFile( fileName );

    while ( finishSpy.count() == 0 ) {
        qApp->processEvents();
    }

    QVERIFY( resultSpy.count() > 0 );
    QCOMPARE( finishSpy.count(), 1 );
}

}

QTEST_MAIN( Marble::MarbleRunnerManagerTest )

#include "MarbleRunnerManagerTest.moc"
