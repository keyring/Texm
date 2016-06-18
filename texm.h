#ifndef TEXM_H
#define TEXM_H

#include <QMainWindow>
#include <QTableWidget>
#include <QProcess>
#include <QFileInfo>

namespace Ui {
class Texm;
}

class Texm : public QMainWindow
{
    Q_OBJECT

public:
    explicit Texm(QWidget *parent = 0);
    ~Texm();

    QTableWidget *file_list_table_widget();
    void file_list_clear();

    QString current_selected_filename();
    void update_file_table();
    void update_big_pixmap();
    void append_file_info_list(const QList<QFileInfo> &info_list);
    void append_file_info_recursive(const QFileInfo &, const int);

protected:
    void set_current_preview_image(const QImage &image);
    void clear_small_preview();
    void clear_big_preview();
    void update_small_preview(const QString &filename);

    QString open_output_directory_pushed(const QString &input_dir);
    bool is_output_directory_valid(const QString &text);
    void output_direction_changed(QLineEdit * const edit_line);

private:
    Ui::Texm *ui;
    QFileInfoList m_file_list;
    QFileInfoList m_file_list_sorted;
    int m_selected_last_row;
    QString m_small_png_filename;
    QImage m_small_png_image;
    QString m_big_png_filename;
    QImage m_big_png_image;
    QPixmap m_big_pixmap;

private slots:
    void menu_open_pushed();
    void menu_exit_pushed();
    void menu_publish_pushed();
    void table_widget_current_changed();
    void output_data_directory_changed();
    void output_texture_directory_changed();
    void open_output_data_directory_pushed();
    void open_output_texture_directory_pushed();
};

#endif // TEXM_H
