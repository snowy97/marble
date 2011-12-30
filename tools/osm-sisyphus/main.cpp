//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2011      Dennis Nienhüser <earthwings@gentoo.org>
//

#include "jobmanager.h"
#include "logger.h"
#include "upload.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>

void usage(const QString &app)
{
    qDebug() << "Usage: " << app << "[options] regions.xml log.sqlite [resume-id]";
    qDebug() << "\nOptions:";
    qDebug() << "\t-h, --help................. Show this help";
    qDebug() << "\t-cd, --cache-downloads..... Do not delete downloaded .osm.pbf files after a successfull conversion and upload";
    qDebug() << "\t-nu, --no-uploads.......... Do not upload converted files to files.kde.org";
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QStringList arguments;
    bool cacheDownloads(false);
    bool uploadFiles(true);
    for (int i=1; i<argc; ++i) {
        QString const arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            usage(argv[0]);
            return 0;
        } else if (arg == "-cd" || arg == "--cache-downloads") {
            cacheDownloads = true;
        } else if (arg == "-nu" || arg == "--no-uploads") {
            uploadFiles = false;
        } else {
            arguments << arg;
        }
    }

    Upload::instance().setCacheDownloads(cacheDownloads);
    Upload::instance().setUploadFiles(uploadFiles);

    if (arguments.size() < 2) {
        usage(argv[0]);
        return 1;
    }

    Logger::instance().setFilename(arguments.at(1));

    JobManager manager;
    manager.setRegionsFile(arguments.at(0));
    if (arguments.size() == 3) {
        manager.setResumeId(arguments.at(2));
    }
    manager.run();
    
    return app.exec();
}