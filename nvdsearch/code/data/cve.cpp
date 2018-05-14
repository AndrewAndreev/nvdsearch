#include "cve.h"

Cve::Cve()
    : _id_cve( 0 ),
      _is_valid( false ),
      _severity( 0.0 ),
      _hasAdditional( false )
{
}

Cve::Cve( int id_cve, QString cve_name, qreal severity, CVSS score_version,
          QDateTime published_date, QDateTime last_modified_date )
    : _id_cve( id_cve ),
      _is_valid( true ),
      _cve_name( std::move( cve_name ) ),
      _severity( severity ),
      _score_version( score_version ),
      _published_date( std::move( published_date ) ),
      _last_modified_date( std::move( last_modified_date ) ),
      _hasAdditional( false )
{
}

Cve::Cve( const Cve &rhs )
    : _id_cve( rhs._id_cve ),
      _is_valid( rhs._is_valid ),
      _cve_name( rhs._cve_name ),
      _severity( rhs._severity ),
      _score_version( rhs._score_version ),
      _published_date( rhs._published_date ),
      _last_modified_date( rhs._last_modified_date ),
      _hasAdditional( rhs._hasAdditional )
{
  _product_names = rhs._product_names;
  _vendor_names = rhs._vendor_names;
  _references = rhs._references;
}

Cve::Cve( Cve &&rhs )
    : _id_cve( rhs._id_cve ),
      _is_valid( rhs._is_valid ),
      _cve_name( std::move( rhs._cve_name ) ),
      _severity( rhs._severity ),
      _score_version( rhs._score_version ),
      _published_date( std::move( rhs._published_date ) ),
      _last_modified_date( std::move( rhs._last_modified_date ) ),
      _hasAdditional( rhs._hasAdditional )
{
  _product_names = std::move( rhs._product_names );
  _vendor_names = std::move( rhs._vendor_names );
  _references = std::move( rhs._references );
}

Cve::~Cve()
{
}

bool Cve::isValid() const
{
  return _is_valid;
}

bool Cve::hasAdditional() const
{
  return _hasAdditional;
}

void Cve::setAdditionalInfo( QStringList &product_names,
                             QStringList &vendor_names,
                             QStringList &references )
{
  _product_names = std::move( product_names );
  _vendor_names = std::move( vendor_names );
  _references = std::move( references );
  _hasAdditional = true;
}

int Cve::id() const
{
  return _id_cve;
}

QString Cve::cveName() const
{
  return _cve_name;
}

qreal Cve::severity() const
{
  return _severity;
}

CVSS Cve::scoreVersion() const
{
  return _score_version;
}

QDateTime Cve::publishedDate() const
{
  return _published_date;
}

QDateTime Cve::lastUpdatedDate() const
{
  return _last_modified_date;
}

QStringList Cve::productNames() const
{
  return _product_names;
}

QStringList Cve::vendorNames() const
{
  return _vendor_names;
}

QStringList Cve::references() const
{
  return _references;
}

Cve &Cve::operator=( const Cve &rhs )
{
  _id_cve = rhs._id_cve;
  _is_valid = rhs._is_valid;
  _cve_name = rhs._cve_name;
  _severity = rhs._severity;
  _score_version = rhs._score_version;
  _published_date = rhs._published_date;
  _last_modified_date = rhs._last_modified_date;
  _hasAdditional = rhs._hasAdditional;
  _product_names = rhs._product_names;
  _vendor_names = rhs._vendor_names;
  _references = rhs._references;
  return *this;
}
