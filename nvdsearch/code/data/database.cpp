#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>
#include <QTextStream>
#include <QTimeZone>
#include "database.h"

Database::Database()
{
  _connection_name = "NVD_database";
}

Database::~Database()
{
}

bool Database::isValid() const
{
  return QSqlDatabase::database( _connection_name ).isOpen();
}

bool Database::setConnection()
{
  QFile config_file( "config.json" );
  if ( config_file.open( QIODevice::ReadOnly | QIODevice::Text ) == false ) {
    qDebug() << "config.json file must present in working directory.";
    return false;
  }

  QTextStream in( &config_file );

  QJsonObject config =
      QJsonDocument::fromJson( in.readAll().toUtf8() ).object();
  QString hostname = config["hostname"].toString();
  QString dbname = config["dbname"].toString();
  QString username = config["username"].toString();
  QString password = config["password"].toString();

  return setConnection( hostname, dbname, username, password );
}

bool Database::setConnection( const QString &hostname, const QString &dbname,
                              const QString &username, const QString &password )
{
  QSqlDatabase db = QSqlDatabase::addDatabase( "QMYSQL", _connection_name );
  db.setHostName( hostname );
  db.setDatabaseName( dbname );
  db.setUserName( username );
  db.setPassword( password );

  if ( isValid() == false )
    qDebug() << db.lastError().text();

  return isValid();
}

Cves Database::getCves( QDateTime first_date, QDateTime last_date,
                        qreal lowest_severity, qreal heighest_severity,
                        CVSS severity_version, QString query_tables,
                        QString query_parameters, int limit )
{
  QSqlDatabase db = QSqlDatabase::database( _connection_name );
  if ( isValid() == false )
    return Cves();

  // Defaults. s_ - string formatted.
  QString s_first_date = "2002-01-01";
  QString s_last_date =
      QDateTime::currentDateTimeUtc().toString( "yyyy-MM-dd HH:mm:ss" );
  QString s_lowest_severity = "0.0";
  QString s_heighest_severity = "10.0";
  QString s_severity_version = "('2', '3')";

  if ( isValidData( first_date ) )
    s_first_date = first_date.toString( "yyyy-MM-dd HH:mm:ss" );
  if ( isValidData( last_date ) )
    s_last_date = last_date.toString( "yyyy-MM-dd HH:mm:ss" );
  if ( isValidData( lowest_severity ) )
    s_lowest_severity = QString( "%1" ).arg( lowest_severity, 0, 'f', 1 );
  if ( isValidData( heighest_severity ) )
    s_heighest_severity = QString( "%1" ).arg( heighest_severity, 0, 'f', 1 );
  if ( isValidData( severity_version ) )
    s_severity_version = QString( "('%1')" ).arg( severity_version );

  QSqlQuery query( db );
  auto s_query =
      QString(
          "select cve.id, cve_name, base_score, version, published_date, "
          "last_modified_date from cve\n"
          "inner join severity on cve.id_severity = severity.id\n"
          "%1\n"
          "where published_date between '%2' and '%3'\n"
          "and severity.version in %4\n"
          "and severity.base_score >= %5 and severity.base_score <= %6\n"
          "%7\n"
          "limit %8;" )
          .arg( query_tables )
          .arg( s_first_date )
          .arg( s_last_date )
          .arg( s_severity_version )
          .arg( s_lowest_severity )
          .arg( s_heighest_severity )
          .arg( query_parameters )
          .arg( limit );
  query.prepare( s_query );

  if ( query.exec() == false ) {
    qDebug() << query.lastError();
    return Cves();
  }

  Cves cve_list;
  while ( query.next() ) {
    auto id = query.value( "id" ).toInt();
    auto cve_name = query.value( "cve_name" ).toString();
    auto base_score = query.value( "base_score" ).toReal();
    auto version = CVSS( query.value( "version" ).toInt() );
    auto published_date = query.value( "published_date" ).toDateTime();
    auto last_modified_date = query.value( "last_modified_date" ).toDateTime();
    published_date.setTimeZone( QTimeZone::utc() );
    last_modified_date.setTimeZone( QTimeZone::utc() );
    cve_list.append( Cve( id, cve_name, base_score, version, published_date,
                          last_modified_date ) );
  }

  return cve_list;
}

Cves Database::getProductCves( QString product_name, QDateTime first_date,
                               QDateTime last_date, qreal lowest_severity,
                               qreal heighest_severity, CVSS severity_version,
                               QString query_parameters, int limit )
{
  auto query_t = QString(
      "inner join product_cve on cve.id = product_cve.id_cve\n"
      "inner join product on product_cve.id_product = product.id" );
  auto query_p = QString( "and product.product_name like '%1'\n%2" )
                     .arg( product_name )
                     .arg( query_parameters );
  return getCves( first_date, last_date, lowest_severity, heighest_severity,
                  severity_version, query_t, query_p, limit );
}

Cves Database::getVendorCves( QString vendor_name, QDateTime first_date,
                              QDateTime last_date, qreal lowest_severity,
                              qreal heighest_severity, CVSS severity_version,
                              QString query_parameters, int limit )
{
  auto query_t = QString(
      "inner join product_cve on cve.id = product_cve.id_cve\n"
      "inner join product on product_cve.id_product = product.id"
      "inner join vendor on product.id_vendor = vendor.id\n" );
  auto query_p = QString( "and product.product_name like '%1'\n%2" )
                     .arg( vendor_name )
                     .arg( query_parameters );
  return getCves( first_date, last_date, lowest_severity, heighest_severity,
                  severity_version, query_t, query_p, limit );
}

bool Database::isValidData( QDateTime datetime ) const
{
  return datetime.isValid() &&
         datetime >= QDateTime::fromString( "2002-01-01", "yyyy-MM-dd" ) &&
         datetime <= QDateTime::currentDateTimeUtc();
}

bool Database::isValidData( qreal severity ) const
{
  return 0.0 <= severity && severity <= 10.0;
}

bool Database::isValidData( CVSS severity_version ) const
{
  return severity_version == CVSS::V2 || severity_version == CVSS::V3;
}
