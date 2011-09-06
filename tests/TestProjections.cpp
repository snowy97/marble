//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2011       Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#include <QtTest/QtTest>
#include "MarbleMap.h"

namespace QTest
{

bool qCompare(qreal val1, qreal val2, qreal epsilon, const char *actual, const char *expected, const char *file, int line)
{
    return ( qAbs( val1 - val2 ) < epsilon )
        ? compare_helper( true, "COMPARE()", file, line )
        : compare_helper( false, "Compared qreals are not the same", toString( val1 ), toString( val2 ), actual, expected, file, line );
}

}

#define QFUZZYCOMPARE(actual, expected, epsilon) \
do {\
    if (!QTest::qCompare(actual, expected, epsilon, #actual, #expected, __FILE__, __LINE__))\
        return;\
} while (0)

namespace Marble
{

class TestProjections : public QObject
{
    Q_OBJECT

 private slots:
    void testAxisSpherical_data();
    void testAxisSpherical();

    void testAxisEquirectangular_data();
    void testAxisEquirectangular();

    void testAxisMercator_data();
    void testAxisMercator();
};

void TestProjections::testAxisSpherical_data()
{
    QTest::addColumn<qreal>( "lon" );
    QTest::addColumn<qreal>( "lat" );

    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) << 0.0 << 0.0;

    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) << -180.0 << 0.0;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<  -90.0 << 0.0;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<   90.0 << 0.0;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<  180.0 << 0.0;

    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) << 0.0 << -90.0;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) << 0.0 << -89.999;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) << 0.0 << -89.0;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) << 0.0 <<  89.0;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) << 0.0 <<  89.999;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) << 0.0 <<  90.0;

    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) << -180.0 << -89.999;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) << -180.0 <<  89.999;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<  -90.0 << -89.999;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<  -90.0 <<  89.999;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<    0.0 << -89.999;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<    0.0 <<  89.999;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<   90.0 << -89.999;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<   90.0 <<  89.999;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<  180.0 << -89.999;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<  180.0 <<  89.999;

    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) << -180.0 << -90.0;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) << -180.0 <<  90.0;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<  -90.0 << -90.0;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<  -90.0 <<  90.0;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<    0.0 << -90.0;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<    0.0 <<  90.0;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<   90.0 << -90.0;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<   90.0 <<  90.0;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<  180.0 << -90.0;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<  180.0 <<  90.0;
}

void TestProjections::testAxisSpherical()
{
    MarbleMap map;

    map.setProjection( Spherical );

    QFETCH( qreal, lon );
    QFETCH( qreal, lat );

    map.centerOn( lon, lat );
    QFUZZYCOMPARE( map.centerLongitude(), lon, 0.0001 );
    QFUZZYCOMPARE( map.centerLatitude(), lat, 0.0001 );
}

void TestProjections::testAxisEquirectangular_data()
{
    QTest::addColumn<qreal>( "lon" );
    QTest::addColumn<qreal>( "lat" );

    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) << 0.0 << 0.0;

    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) << -180.0 << 0.0;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<  -90.0 << 0.0;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<   90.0 << 0.0;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<  180.0 << 0.0;

    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) << 0.0 << -90.0;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) << 0.0 << -89.999;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) << 0.0 << -89.0;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) << 0.0 <<  89.0;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) << 0.0 <<  89.999;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) << 0.0 <<  90.0;

    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) << -180.0 << -89.999;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) << -180.0 <<  89.999;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<  -90.0 << -89.999;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<  -90.0 <<  89.999;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<    0.0 << -89.999;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<    0.0 <<  89.999;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<   90.0 << -89.999;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<   90.0 <<  89.999;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<  180.0 << -89.999;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<  180.0 <<  89.999;

    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) << -180.0 << -90.0;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) << -180.0 <<  90.0;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<  -90.0 << -90.0;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<  -90.0 <<  90.0;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<    0.0 << -90.0;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<    0.0 <<  90.0;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<   90.0 << -90.0;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<   90.0 <<  90.0;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<  180.0 << -90.0;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<  180.0 <<  90.0;
}

void TestProjections::testAxisEquirectangular()
{
    MarbleMap map;

    map.setProjection( Equirectangular );

    QFETCH( qreal, lon );
    QFETCH( qreal, lat );

    map.centerOn( lon, lat );
    QFUZZYCOMPARE( map.centerLongitude(), lon, 0.0001 );
    QFUZZYCOMPARE( map.centerLatitude(), lat, 0.0001 );
}

void TestProjections::testAxisMercator_data()
{
    QTest::addColumn<qreal>( "lon" );
    QTest::addColumn<qreal>( "lat" );
    QTest::addColumn<qreal>( "expectedLat" );

    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) << 0.0 << 0.0 << 0.0;

    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) << -180.0 << 0.0 << 0.0;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<  -90.0 << 0.0 << 0.0;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<   90.0 << 0.0 << 0.0;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<  180.0 << 0.0 << 0.0;

    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) << -180.0 << -85.0511 << -85.0511;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) << -180.0 <<  85.0511 <<  85.0511;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<  -90.0 << -85.0511 << -85.0511;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<  -90.0 <<  85.0511 <<  85.0511;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<    0.0 << -85.0511 << -85.0511;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<    0.0 <<  85.0511 <<  85.0511;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<   90.0 << -85.0511 << -85.0511;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<   90.0 <<  85.0511 <<  85.0511;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<  180.0 << -85.0511 << -85.0511;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<  180.0 <<  85.0511 <<  85.0511;

    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) << -180.0 << -87.0 << -85.0511;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) << -180.0 <<  87.0 <<  85.0511;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<  -90.0 << -87.0 << -85.0511;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<  -90.0 <<  87.0 <<  85.0511;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<    0.0 << -87.0 << -85.0511;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<    0.0 <<  87.0 <<  85.0511;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<   90.0 << -87.0 << -85.0511;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<   90.0 <<  87.0 <<  85.0511;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<  180.0 << -87.0 << -85.0511;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<  180.0 <<  87.0 <<  85.0511;

    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) << -180.0 << -90.0 << -85.0511;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) << -180.0 <<  90.0 <<  85.0511;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<  -90.0 << -90.0 << -85.0511;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<  -90.0 <<  90.0 <<  85.0511;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<    0.0 << -90.0 << -85.0511;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<    0.0 <<  90.0 <<  85.0511;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<   90.0 << -90.0 << -85.0511;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<   90.0 <<  90.0 <<  85.0511;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<  180.0 << -90.0 << -85.0511;
    QTest::newRow( QString( "line %1").arg( __LINE__ ).toAscii().data() ) <<  180.0 <<  90.0 <<  85.0511;
}

void TestProjections::testAxisMercator()
{
    MarbleMap map;

    map.setProjection( Mercator );

    QFETCH( qreal, lon );
    QFETCH( qreal, lat );
    QFETCH( qreal, expectedLat );

    map.centerOn( lon, lat );
    QFUZZYCOMPARE( map.centerLongitude(), lon, 0.0001 );
    QFUZZYCOMPARE( map.centerLatitude(), expectedLat, 0.0001 );
}

}

QTEST_MAIN( Marble::TestProjections )

#include "TestProjections.moc"
