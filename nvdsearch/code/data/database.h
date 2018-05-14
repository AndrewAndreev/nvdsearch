#ifndef DATABASE_H
#define DATABASE_H

#include <QList>
#include <QSqlDatabase>
#include "cve.h"

using Cves = QList<Cve>;

class Database
{
public:
  Database();
  ~Database();

  bool isValid() const;
  bool setConnection();  // credentials from config.json file
  bool setConnection( const QString &hostname, const QString &dbname,
                      const QString &username, const QString &password );

  QString lastError() const;

  // selects CVEs with given filters.
  // Skips filter if invalid value is provided.
  // qreal range = [0.0, 10.0]
  // QDateTime = [2002 year, present day]
  // CVSS = CVSS::V2, CVSS::V3
  // query_tables and query_parameters inserts into query before and after
  // where statement accordingly.
  Cves getCves( QString cve_name_filter = "",
                QDateTime first_date = QDateTime(),
                QDateTime last_date = QDateTime(), qreal lowest_severity = -1.0,
                qreal heighest_severity = -1.0,
                CVSS severity_version = ( CVSS )0, QString query_tables = "",
                QString query_parameters = "", int limit = 1000 );
  // selects CVEs between dates with given filters.
  Cves getProductCves( QString product_name, QDateTime first_date = QDateTime(),
                       QDateTime last_date = QDateTime(),
                       qreal lowest_severity = -1.0,
                       qreal heighest_severity = -1.0,
                       CVSS severity_version = ( CVSS )0,
                       QString query_parameters = "", int limit = 1000 );
  // selects CVEs on products of a given vendor with a given filters.
  Cves getVendorCves( QString vendor_name, QDateTime first_date = QDateTime(),
                      QDateTime last_date = QDateTime(),
                      qreal lowest_severity = -1.0,
                      qreal heighest_severity = -1.0,
                      CVSS severity_version = ( CVSS )0,
                      QString query_parameters = "", int limit = 1000 );
  // selects vendor names of a given CVE id.
  QStringList getCveVendors( QString cve_name, int limit = 1000 );
  // selects product names of a given CVE id.
  QStringList getCveProducts( QString cve_name, QString vendor_name,
                              int limit = 1000 );
  // selects affected versions of a given product and vendor by a given CVE id.
  QStringList getCveProductVersions( QString cve_name, QString vendor_name,
                                     QString product_name, int limit = 1000 );
  // selects first CVE description where lang='en' if there is one.
  QString getCveDescription( QString cve_name );

private:
  // Functions to check validity of passed arguments.
  bool isValidData( QDateTime datetime ) const;
  bool isValidData( qreal severity ) const;
  bool isValidData( CVSS severity_version ) const;

  QString _connection_name;
  QString _last_error;
};

#endif  // !DATABASE_H
