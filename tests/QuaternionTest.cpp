//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2011       Bernhard Beschow <bbeschow@cs.tu-berlin.de>
//

#include <QtCore/QMetaType>
#include <QtTest/QtTest>
#include "Quaternion.h"

Q_DECLARE_METATYPE( Marble::Quaternion )

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

class QuaternionTest : public QObject
{
    Q_OBJECT

 private slots:
    void testLonLat_data();
    void testLonLat();

    void testWXYZ_data();
    void testWXYZ();

    void testFromEuler_data();
    void testFromEuler();

 private:
    static QTestData &addTest( int line );
};

QTestData &QuaternionTest::addTest( int line )
{
    const QString strLine = QString( "line %1" ).arg( line );

    return QTest::newRow( strLine.toAscii().data() );
}

void QuaternionTest::testLonLat_data()
{
    QTest::addColumn<qreal>( "lon" );
    QTest::addColumn<qreal>( "lat" );

    addTest( __LINE__ ) << 0.0 << 0.0;

    addTest( __LINE__ ) << -180.0 << 0.0;
    addTest( __LINE__ ) <<  -90.0 << 0.0;
    addTest( __LINE__ ) <<   90.0 << 0.0;
    addTest( __LINE__ ) <<  180.0 << 0.0;

    addTest( __LINE__ ) << -180.0 << -90.0;
    addTest( __LINE__ ) <<  -90.0 << -90.0;
    addTest( __LINE__ ) <<    0.0 << -90.0;
    addTest( __LINE__ ) <<   90.0 << -90.0;
    addTest( __LINE__ ) <<  180.0 << -90.0;

    addTest( __LINE__ ) << -180.0 << 90.0;
    addTest( __LINE__ ) <<  -90.0 << 90.0;
    addTest( __LINE__ ) <<    0.0 << 90.0;
    addTest( __LINE__ ) <<   90.0 << 90.0;
    addTest( __LINE__ ) <<  180.0 << 90.0;
}

void QuaternionTest::testLonLat()
{
    QFETCH( qreal, lon );
    QFETCH( qreal, lat );

    lon *= M_PI / 180;
    lat *= M_PI / 180;
    Quaternion quat( lon, lat );

    qreal _lon, _lat;
    quat.getSpherical( _lon, _lat );
    qDebug() << lon * 180 / M_PI << lat * 180 / M_PI << quat.v[Q_W] << quat.v[Q_X] << quat.v[Q_Y] << quat.v[Q_Z];

    QFUZZYCOMPARE( _lon, lon, 0.0001 );
    QFUZZYCOMPARE( _lat, lat, 0.0001 );
}

void QuaternionTest::testWXYZ_data()
{
    QTest::addColumn<Quaternion>( "quat" );
    QTest::addColumn<qreal>( "lon" );
    QTest::addColumn<qreal>( "lat" );

    addTest( __LINE__ ) << Quaternion( -1.0, 0.0, 0.0, 0.0 ) <<   0.0 <<   0.0;
    addTest( __LINE__ ) << Quaternion(  1.0, 0.0, 0.0, 0.0 ) <<   0.0 <<   0.0;

    addTest( __LINE__ ) << Quaternion( 0.0, -1.0, 0.0, 0.0 ) << -90.0 <<   0.0;
    addTest( __LINE__ ) << Quaternion( 0.0,  1.0, 0.0, 0.0 ) <<  90.0 <<   0.0;

    addTest( __LINE__ ) << Quaternion( 0.0, 0.0, -1.0, 0.0 ) <<   0.0 << -90.0;
    addTest( __LINE__ ) << Quaternion( 0.0, 0.0,  1.0, 0.0 ) <<   0.0 <<  90.0;

    addTest( __LINE__ ) << Quaternion( 0.0, 0.0, 0.0, -1.0 ) << 180.0 <<   0.0;
    addTest( __LINE__ ) << Quaternion( 0.0, 0.0, 0.0,  1.0 ) <<   0.0 <<   0.0;

    addTest( __LINE__ ) << Quaternion( -1.0, 0.0, 1.0, 0.0 ) <<   0.0 <<  90.0;
    addTest( __LINE__ ) << Quaternion(  1.0, 0.0, 1.0, 0.0 ) <<   0.0 <<  90.0;
}

void QuaternionTest::testWXYZ()
{
    QFETCH( Quaternion, quat );
    QFETCH( qreal, lon );
    QFETCH( qreal, lat );

    lon *= M_PI / 180;
    lat *= M_PI / 180;

    qreal _lon, _lat;
    quat.getSpherical( _lon, _lat );

    QFUZZYCOMPARE( _lon, lon, 0.0001 );
    QFUZZYCOMPARE( _lat, lat, 0.0001 );
}

void QuaternionTest::testFromEuler_data()
{
    QTest::addColumn<Quaternion>( "quat" );
    QTest::addColumn<qreal>( "lon" );
    QTest::addColumn<qreal>( "lat" );

    addTest( __LINE__ ) << Quaternion::fromEuler( -M_PI, 0.0, 0.0 ) << -90.0 <<   0.0;
    addTest( __LINE__ ) << Quaternion::fromEuler(  M_PI, 0.0, 0.0 ) <<  90.0 <<   0.0;

    addTest( __LINE__ ) << Quaternion::fromEuler( 0.0, -M_PI, 0.0 ) <<   0.0 << -90.0;
    addTest( __LINE__ ) << Quaternion::fromEuler( 0.0,  M_PI, 0.0 ) <<   0.0 <<  90.0;

    addTest( __LINE__ ) << Quaternion::fromEuler( 0.0, 0.0, -M_PI ) << 180.0 <<   0.0;
    addTest( __LINE__ ) << Quaternion::fromEuler( 0.0, 0.0,  M_PI ) <<   0.0 <<   0.0;
}

void QuaternionTest::testFromEuler()
{
    QFETCH( Quaternion, quat );
    QFETCH( qreal, lon );
    QFETCH( qreal, lat );

    lon *= M_PI / 180;
    lat *= M_PI / 180;

    qreal _lon, _lat;
    quat.getSpherical( _lon, _lat );

    QFUZZYCOMPARE( _lon, lon, 0.0001 );
    QFUZZYCOMPARE( _lat, lat, 0.0001 );
}

}

QTEST_MAIN( Marble::QuaternionTest )

#include "QuaternionTest.moc"
