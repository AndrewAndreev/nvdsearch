#include <QApplication>
#include "ui/nvdsearchwidget.h"

int main( int argc, char *argv[] )
{
  QApplication::setAttribute( Qt::AA_EnableHighDpiScaling );
  QApplication app( argc, argv );

  NvdSearchWidget widget;
  widget.show();

  return app.exec();
}
