#ifndef TEXM_H
#define TEXM_H

#include <QMainWindow>
#include <QTableWidget>
#include <QProcess>
#include <QFileInfo>

#include "max_rects_pack.h"

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
    void set_busy_mode( const bool b);
    bool is_busy() const;

protected:

    virtual void dragEnterEvent( QDragEnterEvent *event );
    virtual void dragLeaveEvent( QDragLeaveEvent *event );
    virtual void dragMoveEvent( QDragMoveEvent *event );
    virtual void dropEvent( QDropEvent *event );

    void set_current_preview_image(const QImage &image);
    void clear_small_preview();
    void clear_big_preview();
    void update_small_preview(const QString &filename);

    QString open_output_directory_pushed(const QString &input_dir, const char *option);
    bool is_output_directory_valid(const QString &text);
    void output_direction_changed(QLineEdit * const edit_line);

private:
    void update_input_sprites();
    Page *pack_page(std::vector<Sprite> &input_sprites);
    Page *pack_in_size(int width, int height, std::vector<Sprite> &inout_sprites);
    Page *get_best(Page *, Page *);
    void write_data_file(const QString &data_file, const QString &texture_file);
    void write_texture_file(const QString &filename);

private:
    Ui::Texm *ui;
    QFileInfoList m_file_list;
    std::vector<Sprite> m_input_sprites;
    int m_selected_last_row;
    QString m_small_png_filename;
    QImage m_small_png_image;
    QString m_big_png_filename;
    QImage m_big_png_image;
    QPixmap m_big_pixmap;
    Page *m_packed_page;
    bool m_is_busy;

    MaxRectsBinPack m_maxrect_bin;

private slots:
    void menu_open_pushed();
    void menu_exit_pushed();
    void menu_publish_pushed();
    void table_widget_current_changed();
    void output_data_directory_changed();
    void output_texture_directory_changed();
    void open_output_data_directory_pushed();
    void open_output_texture_directory_pushed();
    void on_comboBox_width_currentTextChanged(const QString &arg1);
    void on_comboBox_height_currentTextChanged(const QString &arg1);
};

#endif // TEXM_H
