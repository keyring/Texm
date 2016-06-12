#include "texm.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Texm w;
    w.show();

    return a.exec();
}
