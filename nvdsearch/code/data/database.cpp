#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
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
    _last_error = "There is no config.json file in working directory.";
    qDebug() << _last_error;
    return false;
  }

  QTextStream in( &config_file );

  QJsonParseError err;
  QJsonObject config =
      QJsonDocument::fromJson( in.readAll().toUtf8(), &err ).object();
  if ( err.error != QJsonParseError::NoError ) {
    _last_error = err.errorString();
    qDebug() << _last_error;
    return false;
  }
  if ( config.contains( "hostname" ) == false ||
       config.contains( "dbname" ) == false ||
       config.contains( "username" ) == false ||
       config.contains( "password" ) == false ) {
    _last_error =
        "Wrong config.json file format. Example:\n"
        "{\n"
        "  \"hostname\": \"127.0.0.1\",\n"
        "  \"dbname\": \"nvdb\",\n"
        "  \"username\": \"qt_user\",\n"
        "  \"password\": \"0000\"\n"
        "}\n";
    qDebug() << _last_error;
    return false;
  }

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

  if ( isValid() == false ) {
    _last_error = db.lastError().text();
    qDebug() << _last_error;
  }

  return isValid();
}

QString Database::lastError() const
{
  return _last_error;
}

Cves Database::getCves( QString cve_name_filter, QDateTime first_date,
                        QDateTime last_date, qreal lowest_severity,
                        qreal heighest_severity, CVSS severity_version,
                        QString query_tables, QString query_parameters,
                        int limit )
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
          "and cve_name like '%7'\n"
          "%8\n"
          "limit %9;" )
          .arg( query_tables )
          .arg( s_first_date )
          .arg( s_last_date )
          .arg( s_severity_version )
          .arg( s_lowest_severity )
          .arg( s_heighest_severity )
          .arg( "%" + cve_name_filter + "%" )
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
      "inner join product on product_cve.id_product = product.id\n" );
  auto query_p = QString( "and product.product_name like '%1'\n%2" )
                     .arg( product_name )
                     .arg( query_parameters );
  return getCves( "", first_date, last_date, lowest_severity, heighest_severity,
                  severity_version, query_t, query_p, limit );
}

Cves Database::getVendorCves( QString vendor_name, QDateTime first_date,
                              QDateTime last_date, qreal lowest_severity,
                              qreal heighest_severity, CVSS severity_version,
                              QString query_parameters, int limit )
{
  auto query_t = QString(
      "inner join product_cve on cve.id = product_cve.id_cve\n"
      "inner join product on product_cve.id_product = product.id\n"
      "inner join vendor on product.id_vendor = vendor.id\n" );
  auto query_p = QString( "and product.product_name like '%1'\n%2" )
                     .arg( vendor_name )
                     .arg( query_parameters );
  return getCves( "", first_date, last_date, lowest_severity, heighest_severity,
                  severity_version, query_t, query_p, limit );
}

QStringList Database::getCveVendors( QString cve_name, int limit )
{
  QSqlDatabase db = QSqlDatabase::database( _connection_name );
  if ( isValid() == false )
    return QStringList();

  QSqlQuery query( db );
  auto s_query =
      QString(
          "select distinct vendor_name from vendor\n"
          "inner join product on vendor.id = product.id_vendor\n"
          "inner join product_cve on product.id = product_cve.id_product\n"
          "inner join cve on product_cve.id_cve = cve.id\n"
          "where\n"
          "cve_name = '%1'\n"
          "limit %2;\n" )
          .arg( cve_name )
          .arg( limit );
  query.prepare( s_query );

  if ( query.exec() == false ) {
    qDebug() << query.lastError();
    return QStringList();
  }

  QStringList vendor_list;
  while ( query.next() ) {
    auto vendor_name = query.value( "vendor_name" ).toString();
    vendor_list.append( vendor_name );
  }

  return vendor_list;
}

QStringList Database::getCveProducts( QString cve_name, QString vendor_name,
                                      int limit )
{
  QSqlDatabase db = QSqlDatabase::database( _connection_name );
  if ( isValid() == false )
    return QStringList();

  QSqlQuery query( db );
  auto s_query =
      QString(
          "select distinct product_name from product\n"
          "inner join vendor on product.id_vendor = vendor.id\n"
          "inner join product_cve on product.id = product_cve.id_product\n"
          "inner join cve on product_cve.id_cve = cve.id\n"
          "where\n"
          "cve_name = '%1'\n"
          "and vendor_name = '%2'\n"
          "limit %3;\n" )
          .arg( cve_name )
          .arg( vendor_name )
          .arg( limit );
  query.prepare( s_query );

  if ( query.exec() == false ) {
    qDebug() << query.lastError();
    return QStringList();
  }

  QStringList product_list;
  while ( query.next() ) {
    auto product_name = query.value( "product_name" ).toString();
    product_list.append( product_name );
  }

  return product_list;
}

QStringList Database::getCveProductVersions( QString cve_name,
                                             QString vendor_name,
                                             QString product_name, int limit )
{
  QSqlDatabase db = QSqlDatabase::database( _connection_name );
  if ( isValid() == false )
    return QStringList();

  QSqlQuery query( db );
  auto s_query =
      QString(
          "select distinct version_value from version\n"
          "inner join product_cve_version on version.id = "
          "product_cve_version.id_version\n"
          "inner join product_cve on product_cve_version.id_product_cve = "
          "product_cve.id\n"
          "inner join product on product_cve.id_product = product.id\n"
          "inner join vendor on product.id_vendor = vendor.id\n"
          "inner join cve on product_cve.id_cve = cve.id\n"
          "where \n"
          "cve_name = '%1'\n"
          "and vendor_name = '%2'\n"
          "and product_name = '%3'\n"
          "limit %4;\n" )
          .arg( cve_name )
          .arg( vendor_name )
          .arg( product_name )
          .arg( limit );
  query.prepare( s_query );

  if ( query.exec() == false ) {
    qDebug() << query.lastError();
    return QStringList();
  }

  QStringList version_list;
  while ( query.next() ) {
    auto version_value = query.value( "version_value" ).toString();
    version_list.append( version_value );
  }

  return version_list;
}

QString Database::getCveDescription( QString cve_name )
{
  QSqlDatabase db = QSqlDatabase::database( _connection_name );
  if ( isValid() == false )
    return QString();

  QSqlQuery query( db );
  auto s_query = QString(
                     "select `value`, `lang` from description\n"
                     "inner join cve on description.id_cve = cve.id\n"
                     "where\n"
                     "cve_name = '%1'\n"
                     "limit %2;\n" )
                     .arg( cve_name )
                     .arg( 1000 );
  query.prepare( s_query );

  if ( query.exec() == false ) {
    qDebug() << query.lastError();
    return QString();
  }

  QString description;
  while ( query.next() ) {
    auto value = query.value( "value" ).toString();
    auto lang = query.value( "lang" ).toString();
    if ( lang == "en" ) {
      description = std::move( value );
      break;
    }
  }

  return description;
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
