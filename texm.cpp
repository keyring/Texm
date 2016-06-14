#include "texm.h"
#include "ui_texm.h"

#include <cmath>

#include <QProcess>
#include <QDebug>
#include <QTime>
#include <QDialogButtonBox>

#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>

#include <QMoveEvent>

#include <QDir>
#include <QMimeData>
#include <QUrl>
#include <QFileInfo>
#include <QFileDialog>
#include <QDesktopWidget>
#include <QSettings>

#include <QImageReader>
#include <QMessageBox>

#include "texm_util.h"

Texm::Texm(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Texm),
    m_selected_last_row(-1)
{
    ui->setupUi(this);

    {
     QTableWidget *table_widget = file_list_table_widget();
     table_widget->setColumnCount(2);
     table_widget->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("File Name")));
     table_widget->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Size")));
    }

    { // init small image view
      QPalette p = ui->smallImagePreview->palette();
      p.setBrush( QPalette::Base, QBrush(QColor("white")) );
      ui->smallImagePreview->setPalette( p );
      ui->smallImagePreview->setBackgroundBrush( QBrush( QPixmap(":background/checkerboard.png") ) );
    }

    { // init big image view
      QPalette p = ui->bigImagePreview->palette();
      p.setBrush( QPalette::Base, QBrush(QColor(50,50,50)) );
      ui->bigImagePreview->setPalette( p );
      ui->bigImagePreview->setBackgroundBrush( QBrush( QPixmap(":background/checkerboard.png") ) );
    }

    ////
    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(menu_open_pushed()));
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(menu_exit_pushed()));
    connect(ui->actionPublish, SIGNAL(triggered()), this, SLOT(menu_publish_pushed()));

    table_widget_current_changed();

}

Texm::~Texm()
{
    delete ui;
}

QTableWidget * Texm::file_list_table_widget()
{
    return ui->tableWidget_fileList;
}

void Texm::file_list_clear()
{
    m_file_list.clear();
    update_file_table();
}

QString Texm::current_selected_filename()
{
    QTableWidget * const table_widget = file_list_table_widget();
    const int current_row = table_widget->currentRow();
    const QTableWidgetItem *const name_item = table_widget->item(current_row, 0);
    return name_item ? name_item->text() : QString();
}

void Texm::update_big_pixmap()
{
    m_big_pixmap = QPixmap(2048, 2048);
    foreach (const QFileInfo &file_info, m_file_list) {

    }
}

void Texm::update_file_table()
{
    QTableWidget *table_widget = file_list_table_widget();
    disconnect(table_widget, SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(table_widget_current_changed()));
    const QString &last_selected_filename = current_selected_filename();
    table_widget->setRowCount(m_file_list.size());

    int row_index = 0;
    foreach (const QFileInfo &file_info, m_file_list) {
        QTableWidgetItem *const filename_item = new QTableWidgetItem(file_info.fileName());
        filename_item->setToolTip(file_info.fileName());
        table_widget->setItem(row_index, 0, filename_item);

        QTableWidgetItem *const origin_size_item =
            new QTableWidgetItem(TexmUtil::size_to_string_kb( file_info.size() ));
        origin_size_item->setData(1, static_cast<double>(file_info.size()) );
        table_widget->setItem( row_index, 1, origin_size_item );

        if(last_selected_filename == file_info.fileName()){
            table_widget->setCurrentItem(filename_item);
        }

        ++row_index;
    }

    connect( table_widget, SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(table_widget_current_changed()) );
    table_widget_current_changed();
}

void Texm::append_file_info_list(const QList<QFileInfo> &info_list)
{
    foreach (const QFileInfo &info, info_list) {
        append_file_info_recursive(info, 0);
    }

    update_file_table();
}

void Texm::append_file_info_recursive(const QFileInfo &file_info, const int depth)
{
    QApplication::processEvents();

    if(file_info.isFile()){
        const bool is_png = TexmUtil::has_png_extension(file_info);
        if(is_png && !m_file_list.contains(file_info)){
            m_file_list.push_back(file_info);
        }
    }else if(file_info.isDir()){
        const QDir dir(file_info.absoluteFilePath());
        foreach (const QFileInfo &child_file_info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries)) {
            append_file_info_recursive(child_file_info, depth+1);
        }
    }
}


void Texm::set_png_file(const QString &filename)
{

}

void Texm::load_png_file()
{

}

void Texm::set_current_preview_image(const QImage &image)
{
    ui->smallImagePreview->setImage(image);
    ui->smallImagePreview->repaint();
}

void Texm::clear_small_preview()
{
    m_small_png_filename = QString();
    m_small_png_image = QImage();
    set_current_preview_image(QImage());
}

void Texm::clear_big_preview()
{

}

void Texm::update_small_preview(const QString &filename)
{
    if(m_small_png_filename == filename){
        return;
    }
    clear_small_preview();
    m_small_png_filename = filename;
    m_small_png_image = QImage(m_small_png_filename);

    set_current_preview_image(m_small_png_image);

    QString msg = QString("%1   %2x%3").arg(filename).arg(m_small_png_image.width()).arg(m_small_png_image.height());
    ui->statusbar->showMessage(msg);

}

void Texm::update_big_preview(const QString &filename)
{

}


//// Slots

void Texm::menu_open_pushed()
{
    const QStringList filepath_list = QFileDialog::getOpenFileNames(
                this, QString(), QDir::homePath(), "PNG file (*.png);;");

    if(filepath_list.empty())
        return;

    QList<QFileInfo> fileinfo_list;
    foreach (const QString &path, filepath_list) {
        fileinfo_list.push_back(QFileInfo(path));
    }
    append_file_info_list(fileinfo_list);
}

void Texm::menu_exit_pushed()
{
    close();
}

void Texm::menu_publish_pushed()
{

}

void Texm::table_widget_current_changed()
{
    // 点击即可预览该图片
    QTableWidget * const table_widget = file_list_table_widget();
    const int current_row = table_widget->currentRow();

    if(current_row < 0 || m_selected_last_row == current_row){
        return;
    }
//    qDebug() << current_row;
    m_selected_last_row = current_row;
    QFileInfo file_info = m_file_list.at(current_row);
    const QString current_path = file_info.absoluteFilePath();

    update_small_preview(current_path);
}
