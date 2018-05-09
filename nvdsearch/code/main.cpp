#include <QApplication>

int main( int argc, char *argv[] )
{
  QApplication::setAttribute( Qt::AA_EnableHighDpiScaling );
  QApplication app( argc, argv );

  return app.exec();
}
