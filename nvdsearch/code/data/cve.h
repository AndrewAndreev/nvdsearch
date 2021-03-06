#ifndef CVE_DATATYPES_H
#define CVE_DATATYPES_H

#include <QDateTime>
#include <QString>

enum CVSS { ANY = 0, V2 = 2, V3 = 3 };

class Cve
{
public:
  Cve();
  Cve( int id_cve, QString cve_name, qreal severity, CVSS score_version,
       QDateTime published_date, QDateTime last_modified_date );
  Cve( const Cve &rhs );
  Cve( Cve &&rhs );
  ~Cve();

  bool isValid() const;
  bool hasAdditional() const;

  void setAdditionalInfo( QStringList &product_names, QStringList &vendor_names,
                          QStringList &references );

  int id() const;
  QString cveName() const;
  qreal severity() const;
  CVSS scoreVersion() const;
  QDateTime publishedDate() const;
  QDateTime lastUpdatedDate() const;
  QStringList productNames() const;
  QStringList vendorNames() const;
  QStringList references() const;

  Cve &operator=( const Cve &rhs );

private:
  // Basic CVE info.
  int _id_cve;
  bool _is_valid;

  QString _cve_name;
  qreal _severity;
  CVSS _score_version;

  QDateTime _published_date;
  QDateTime _last_modified_date;

  // Additional CVE info.
  QStringList _product_names;
  QStringList _vendor_names;
  QStringList _references;
  bool _hasAdditional;
};

#endif  // !CVE_DATATYPES_H
