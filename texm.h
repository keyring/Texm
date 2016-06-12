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

    QString current_selected_filename();
    void table_widget_current_changed();
    void update_file_table();
    void append_file_info_list(const QList<QFileInfo> &info_list);
    void append_file_info_recursive(const QFileInfo &, const int);

private:
    Ui::Texm *ui;
    QFileInfoList m_file_list;

private slots:
    void menu_open_pushed();
    void menu_exit_pushed();
    void menu_publish_pushed();
};

#endif // TEXM_H
