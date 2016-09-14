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


Texm::Texm(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Texm),
    m_selected_last_row(-1),
    m_is_busy( false ),
    m_packed_page(nullptr)
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
      p.setBrush( QPalette::Base, QBrush(QColor("white")) );
      ui->bigImagePreview->setPalette( p );
      ui->bigImagePreview->setBackgroundBrush( QBrush( QPixmap(":background/checkerboard.png") ) );
    }

    {
        QRegExp rx("^[0-9]{1,10}$");//这个10就是最大长度
        QValidator *validator = new QRegExpValidator(rx,0);

        ui->comboBox_width->lineEdit()->setValidator(validator);
        ui->comboBox_height->lineEdit()->setValidator(validator);
    }

    ////
    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(menu_open_pushed()));
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(menu_exit_pushed()));
    connect(ui->actionPublish, SIGNAL(triggered()), this, SLOT(menu_publish_pushed()));
    connect( ui->lineEdit_output_data, SIGNAL(textChanged(QString)), this, SLOT(output_data_directory_changed()) );
    connect( ui->lineEdit_output_texture, SIGNAL(textChanged(QString)), this, SLOT(output_texture_directory_changed()) );
    connect( ui->toolButton_output_data, SIGNAL(clicked()), this, SLOT(open_output_data_directory_pushed()) );
    connect( ui->toolButton_output_texture, SIGNAL(clicked()), this, SLOT(open_output_texture_directory_pushed()) );
    connect(ui->comboBox_width, SIGNAL(currentTextChanged(QString)), this, SLOT(on_comboBox_width_currentTextChanged(QString)));
    connect(ui->comboBox_height, SIGNAL(currentTextChanged(QString)), this, SLOT(on_comboBox_height_currentTextChanged(QString)));


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
    m_input_sprites.clear();
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
    Page *packed_page = pack_page(m_input_sprites);

    if(packed_page == NULL){
        qDebug() << packed_page->width << packed_page->height;
    }

    QMatrix matrix;
    matrix.rotate(90.0);
    m_big_pixmap = QPixmap(packed_page->width, packed_page->height);
    m_big_pixmap.fill(Qt::transparent);
    QPainter paint;

    paint.begin(&m_big_pixmap);

    foreach (const Sprite &sprite, packed_page->output_sprites) {
        if(sprite.height <= 0){
            qDebug() << "Failed! Could not find a proper position to pack this rectangle into. Skipping this one. " ;
            continue;
        }
        const QString name = QString::fromStdString(sprite.sprite_name);
        qDebug() << name  ;
        QImage image = QImage(name);
        if(image.width() != sprite.width){
            image = image.transformed(matrix, Qt::FastTransformation);
        }

        QRect target_rect = QRect(sprite.x, sprite.y, sprite.width, sprite.height);
        paint.drawImage(target_rect, image);
    }

    paint.end();

    ui->bigImagePreview->setPixmap(m_big_pixmap);

    if (m_packed_page != nullptr)
        delete(m_packed_page);

     m_packed_page = packed_page;

}

void Texm::update_input_sprites()
{
    qSort(m_input_sprites.begin(), m_input_sprites.end(), [](Sprite &a, Sprite &b){
        return a.width > b.width;
    });    // sort first is best
}

Page *Texm::pack_page(std::vector<Sprite> &input_sprites)
{
    /* setting replace it */
    int max_width = ui->comboBox_width->currentText().toInt();
    int max_height = ui->comboBox_height->currentText().toInt();
    bool rotate = true;

    int min_width =INT_MAX,  min_height = INT_MAX;

    foreach (const Sprite &sprite, input_sprites) {
        int w = sprite.width;
        int h = sprite.height;
        if( rotate ? ((w>max_width || h>max_height) && (w>max_height || h>max_width) )
                : (w>max_width || h>max_height) ){
            qDebug() << "image does not fit max page size";
            return NULL;  // image does not fit max page size
        }
        min_width = std::min(min_width, w);
        min_height = std::min(min_height, h);
    }


    Page *best_result = NULL;

    BinarySearch *width_search = new BinarySearch(min_width, max_width, 15);
    BinarySearch *height_search = new BinarySearch(min_height, max_height, 15);
    int width = width_search->reset();
    int height = height_search->reset();
    qDebug() << "image does not fit max page size"<< width << height;
    int i = 0;
    while(true){
        Page *best_width_result = NULL;
        while(width != -1){

            Page *result = pack_in_size(width, height, input_sprites);
            if(++i % 50 == 0) qDebug() << "#";
            best_width_result = get_best(best_width_result, result);
            width = width_search->next(result==NULL);

        }
        best_result = get_best(best_result, best_width_result);
        height = height_search->next(best_width_result==NULL);
        if(height == -1) break;
        width = width_search->reset();
    }

    delete(width_search);
    delete(height_search);

    return best_result;

}

Page *Texm::pack_in_size(int width, int height, std::vector<Sprite> &inout_sprites)
{
    Page *best_result= NULL;
    std::vector<Sprite> unused;
    for(int i = 0; i < 5; ++i ){
        m_maxrect_bin.Init(width, height, true);
        Page *result = NULL;

        foreach (const Sprite &sprite, inout_sprites) {
            Sprite s = m_maxrect_bin.Insert(sprite, FreeRectChoiceHeuristic(i));
            if(s.height == 0){
                unused.push_back(s);
            }
        }
        result = m_maxrect_bin.Result();
        if(unused.size() > 0){
            unused.clear();
            continue;
        }
        if(result->output_sprites.size() == 0) continue;
        best_result = get_best(best_result, result);
        qDebug() << best_result->width << best_result->height ;
    }

    return best_result;
}

Page *Texm::get_best(Page *result1, Page *result2)
{
    if(result1 == NULL) return result2;
    if(result2 == NULL) return result1;
    if(result1->occupancy > result2->occupancy){
        delete(result2);
        return result1;
    }else{
        delete(result1);
        return result2;
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
    update_big_pixmap();
}

void Texm::append_file_info_list(const QList<QFileInfo> &info_list)
{
    if(is_busy()){
        return;
    }
    set_busy_mode( true );
    foreach (const QFileInfo &info, info_list) {
        append_file_info_recursive(info, 0);
    }

    update_input_sprites();
    update_file_table();
    set_busy_mode( false );
}

void Texm::append_file_info_recursive(const QFileInfo &file_info, const int depth)
{
    QApplication::processEvents();

    if(file_info.isFile()){
        const bool is_png = TexmUtil::has_png_extension(file_info);
        if(is_png && !m_file_list.contains(file_info)){
            m_file_list.push_back(file_info);
            const QString current_path = file_info.absoluteFilePath();
            QImage image = QImage(current_path);
            Sprite sprite;
            sprite.sprite_name = current_path.toStdString();
            sprite.x = 0;
            sprite.y = 0;
            sprite.width = image.width();
            sprite.height = image.height();
            m_input_sprites.push_back(sprite);
        }
    }else if(file_info.isDir()){
        const QDir dir(file_info.absoluteFilePath());
        foreach (const QFileInfo &child_file_info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries)) {
            append_file_info_recursive(child_file_info, depth+1);
        }
    }
}

void Texm::set_busy_mode( const bool b)
{
    m_is_busy = b;
}

bool Texm::is_busy() const
{
    return m_is_busy;
}


//////////////////////
// protected functions
//////////////////////

void Texm::dragEnterEvent( QDragEnterEvent *event )
{
  if( is_busy() )
  {
    return;
  }

  if( event->mimeData()->hasUrls() )
  {
    event->accept();
  }
}

void Texm::dragLeaveEvent( QDragLeaveEvent *event )
{
  Q_UNUSED(event)
  if( is_busy() )
  {
    return;
  }
}


void Texm::dragMoveEvent( QDragMoveEvent * event )
{
  Q_UNUSED(event)
  if( is_busy() )
  {
    return;
  }

}

void Texm::dropEvent( QDropEvent *event )
{
  if( is_busy() )
  {
    return;
  }


  QWidget * const central_widget = ui->centralwidget;
  const bool mouse_is_on_image_view = TexmUtil::is_under_mouse( central_widget );

  const QMimeData * const mimedata = event->mimeData();
  if( ! mimedata->hasUrls() )
  {
    return;
  }

  const QList<QUrl> &url_list = mimedata->urls();

  if( mouse_is_on_image_view )
  { // append png files
    QList<QFileInfo> file_list;
    foreach( const QUrl &url, url_list )
    {
      file_list.push_back( QFileInfo( url.toLocalFile() ) );
    }
    append_file_info_list( file_list );
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


QString Texm::open_output_directory_pushed(const QString &inputted_dir_path, const char *option)
{
  const QString &current_dir =
          inputted_dir_path.isEmpty() ? QString() : QDir( inputted_dir_path ).absolutePath();
  const QString default_dir =
      QFile::exists(current_dir) ? current_dir : QDir::homePath();

  const QString path = QFileDialog::getSaveFileName(
        this,
        tr("Save"),
        default_dir,
        tr(option));

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

void Texm::write_data_file(const QString &data_file_name, const QString &texture_file_name)
{
    QFile f(data_file_name);
    if(!f.open((QIODevice::WriteOnly | QIODevice::Text))){
        return;
    }

    QString table_name = QFileInfo(data_file_name).baseName().append("_map");
    QString texture_name = QFileInfo(texture_file_name).fileName();

    QTextStream data_out(&f);

    data_out << "-- created with Texm" << endl;
    data_out << "local path = \"" << texture_name <<"\"" << endl;
    data_out << table_name << " = {" << endl << endl;

    foreach (const Sprite &sprite, m_packed_page->output_sprites) {
        if(sprite.height <= 0){
            qDebug() << "Failed! Could not find a proper position to pack this rectangle into. Skipping this one. " ;
            continue;
        }
        const QString name = QFileInfo(QString::fromStdString(sprite.sprite_name)).fileName();
        qDebug() << name  ;

        data_out << "\t[\"" << name << "\"] = {" << endl;
        data_out << "\t\tfile = path," << endl;
        data_out << "\t\tx=" << sprite.x << ",y=" << sprite.y << "," << endl;
        data_out << "\t\twidth=" << sprite.width << ",height=" << sprite.height << endl << "\t}," << endl;

    }

    data_out << "}" << endl;

    f.close();

}

void Texm::write_texture_file(const QString &filename)
{
    m_big_pixmap.save(filename, "png");
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
    QString &texture_file_name = ui->lineEdit_output_texture->text();
    if(texture_file_name.isEmpty())
        return;
    if(QFileInfo(texture_file_name).suffix().isEmpty()){
        texture_file_name.append(".png");
    }
    write_texture_file(texture_file_name);

    QString &data_file_name = ui->lineEdit_output_data->text();
    if(data_file_name.isEmpty())
        return;
    if(QFileInfo(data_file_name).suffix().isEmpty()){
        data_file_name.append(".lua");
    }
    write_data_file(data_file_name, texture_file_name);
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
    QString &output_dir_path = open_output_directory_pushed(ui->lineEdit_output_data->text(), "*.lua");
    if(output_dir_path.isEmpty()){
        return;
    }

    if(QFileInfo(output_dir_path).suffix().isEmpty()){
        output_dir_path.append(".lua");
    }

    ui->lineEdit_output_data->setText(output_dir_path);
    ui->lineEdit_output_texture->setText(QFileInfo(output_dir_path).absolutePath() + "/" + QFileInfo(output_dir_path).baseName() + ".png");
}

void Texm::open_output_texture_directory_pushed()
{
    QString &output_dir_path = open_output_directory_pushed(ui->lineEdit_output_texture->text(), "*.png");
    if(output_dir_path.isEmpty()){
        return;
    }
    if(QFileInfo(output_dir_path).suffix().isEmpty()){
        output_dir_path.append(".png");
    }
    ui->lineEdit_output_texture->setText(output_dir_path);
}



void Texm::on_comboBox_width_currentTextChanged(const QString &arg1)
{
   // update_big_pixmap();
}

void Texm::on_comboBox_height_currentTextChanged(const QString &arg1)
{
    //update_big_pixmap();
}
