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

inline bool is_under_mouse( const QWidget * const widget )
{
  return QRect( QPoint(), widget->size() ).contains( widget->mapFromGlobal( QCursor::pos() ) );
}

inline void set_drop_here_stylesheet(
    QWidget * const widget,
    const bool drag_hoverring,
    const QColor &hoverring_color = QColor("lightskyblue") )
{
//  widget->setStyleSheet();
  QString stylesheet =
      "QWidget{"
      "background-image : url(:/background/drop_here.png);"
      "background-position: center ;"
      "background-repeat : repeat-none;";
  if( drag_hoverring )
  {
    stylesheet += "background-color : " + hoverring_color.name() + ";\n";
  }
  else
  {
    stylesheet += "background-color : " + QColor("white").name() + ";\n";
  }
  stylesheet += "}";

  widget->setStyleSheet( stylesheet );
}

}

#endif // TEXM_UTIL_H
