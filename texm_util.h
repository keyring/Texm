#ifndef TEXM_UTIL_H
#define TEXM_UTIL_H
#include <cmath>

#include <QWidget>
#include <QApplication>
#include <QCursor>
#include <QFileInfo>
#include <QImageReader>
#include <QRegExp>
#include <QDir>
#include <QFile>
#include <QByteArray>
#include <QStringList>
#include <QProcess>
#include <QIcon>

namespace TexmUtil {

inline bool has_png_extension(const QFileInfo &file){
    return QRegExp("png", Qt::CaseInsensitive).exactMatch(file.suffix());
}

inline QString size_to_string_kb(const qint64 size){
    return QString( "%1KB" )
        .arg( static_cast<double>(size) / 1024, 0, 'f', 2 );
}

inline QString size_to_string_mb( const qint64 size ){
  return QString( "%1MB" )
      .arg( static_cast<double>(size) / ( 1024 * 1024 ), 0, 'f', 2 );
}



}

#endif // TEXM_UTIL_H
