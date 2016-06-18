#include "texm.h"
#include "ui_texm.h"

#include <cmath>
#include <QtAlgorithms>

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
#include <QPainter>
#include <QMatrix>
#include <QMessageBox>

#include "texm_util.h"
#include "max_rects_pack.h"

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
    connect( ui->lineEdit_output_data, SIGNAL(textChanged(QString)), this, SLOT(output_data_directory_changed()) );
    connect( ui->lineEdit_output_texture, SIGNAL(textChanged(QString)), this, SLOT(output_texture_directory_changed()) );
    connect( ui->toolButton_output_data, SIGNAL(clicked()), this, SLOT(open_output_data_directory_pushed()) );
    connect( ui->toolButton_output_texture, SIGNAL(clicked()), this, SLOT(open_output_texture_directory_pushed()) );

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
    m_file_list_sorted.clear();
    update_file_table();
}

QString Texm::current_selected_filename()
{
    QTableWidget * const table_widget = file_list_table_widget();
    const int current_row = table_widget->currentRow();
    const QTableWidgetItem *const name_item = table_widget->item(current_row, 0);
    return name_item ? name_item->text() : QString();
}

static bool sort_file_list(const QFileInfo &a, const QFileInfo &b)
{
    return a.size() > b.size();
}

void Texm::update_big_pixmap()
{
    /*
        RectBestShortSideFit, ///< -BSSF: Positions the rectangle against the short side of a free rectangle into which it fits the best.
        RectBestLongSideFit, ///< -BLSF: Positions the rectangle against the long side of a free rectangle into which it fits the best.
        RectBestAreaFit, ///< -BAF: Positions the rectangle into the smallest free rect into which it fits.
        RectBottomLeftRule, ///< -BL: Does the Tetris placement.
        RectContactPointRule ///< -CP: Choosest the placement where the rectangle touches other rects as much as possible.
    */
    QMatrix matrix;
    matrix.rotate(90.0);
    m_big_pixmap = QPixmap(256, 512);
    QPainter paint;
    rbp::MaxRectsBinPack bin_pack;
    rbp::MaxRectsBinPack::FreeRectChoiceHeuristic heuristic = rbp::MaxRectsBinPack::RectContactPointRule;

    bin_pack.Init(256, 512, true);

    m_file_list_sorted = m_file_list;

    qSort(m_file_list_sorted.begin(), m_file_list_sorted.end(), sort_file_list);

    paint.begin(&m_big_pixmap);

    // sort first is best
    foreach (const QFileInfo &file_info, m_file_list_sorted) {
        qDebug() << file_info.size();
        const QString current_path = file_info.absoluteFilePath();
        QImage image = QImage(current_path);
        const rbp::Rect packed_rect = bin_pack.Insert(image.width()+2, image.height()+2, heuristic);

        if(packed_rect.height <= 0){
            qDebug() << "Failed! Could not find a proper position to pack this rectangle into. Skipping this one. " << current_path ;
            continue;
        }

        if(packed_rect.width != image.width()+2){
            image = image.transformed(matrix, Qt::FastTransformation);
        }

        QRect target_rect = QRect(packed_rect.x+1, packed_rect.y+1, packed_rect.width-2, packed_rect.height-2);
        paint.drawImage(target_rect, image);
    }
    paint.end();

    ui->bigImagePreview->setPixmap(m_big_pixmap);
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

    update_big_pixmap();
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
    ui->bigImagePreview->setImage(QImage());
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


QString Texm::open_output_directory_pushed(const QString &inputted_dir_path)
{
  const QString &current_dir =
          inputted_dir_path.isEmpty() ? QString() : QDir( inputted_dir_path ).absolutePath();
  const QString default_dir =
      QFile::exists(current_dir) ? current_dir : QDir::homePath();

  const QString path = QFileDialog::getSaveFileName(
        this,
        tr("Save"),
        QString(),
        tr("*.") );

    return path;
}

bool Texm::is_output_directory_valid(const QString &text)
{
    const QFileInfo output_directory(text);
    return output_directory.isFile();
}

void Texm::output_direction_changed(QLineEdit *const line_edit)
{
    QString &text = line_edit->text();
    QPalette palette = line_edit->palette();
    if(!text.isEmpty() || is_output_directory_valid(text))
    {
      palette.setBrush( QPalette::Text, QBrush() );
    }
    else
    { // if output directory is invalid, change text color
      palette.setBrush( QPalette::Text, QBrush(Qt::red) );
    }
    line_edit->setPalette( palette );
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
    QString &filename = ui->lineEdit_output_texture->text();
    if(filename.isEmpty())
        return;
    m_big_pixmap.save(filename, "png");
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

void Texm::output_data_directory_changed()
{
    output_direction_changed(ui->lineEdit_output_data);

}

void Texm::output_texture_directory_changed()
{
    output_direction_changed(ui->lineEdit_output_texture);
}

void Texm::open_output_data_directory_pushed()
{
    const QString &output_dir_path = open_output_directory_pushed(ui->lineEdit_output_data->text());
    ui->lineEdit_output_data->setText(output_dir_path);
}

void Texm::open_output_texture_directory_pushed()
{
    const QString &output_dir_path = open_output_directory_pushed(ui->lineEdit_output_texture->text());
    ui->lineEdit_output_texture->setText(output_dir_path);
}


