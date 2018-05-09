#include <QApplication>
#include "ui/cvesearchwidget.h"

int main( int argc, char *argv[] )
{
  QApplication::setAttribute( Qt::AA_EnableHighDpiScaling );
  QApplication app( argc, argv );

  Database database;
  database.setConnection();
  CveSearchWidget widget( nullptr, database );
  widget.show();

  return app.exec();
}
