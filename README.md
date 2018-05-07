[![Build Status](https://travis-ci.com/AndrewAndreev/nvdsearch.svg?branch=master)](https://travis-ci.com/AndrewAndreev/nvdsearch)
# nvdsearch

GUI application to search National Vulnerability Database (NVD) locally for Vulnerabilities (CVEs). 
Application is QML-based and requires Qt 5.10

## Requirements
* Qt 5.10
* MySQL 

# Database

MySQL database should has the following structure:

![stracture](https://github.com/AndrewAndreev/nvdsearch/blob/master/nvdsearch/tools/db.png)

File `nvdsearch/tools/db_structure.sql` contains SQL script to create the above structure.

To download and feed database with data, run python scripts:
* `python nvdsearch/tools/nvd.py` - it will download all database files from NVD.
* `python nvdsearch/tools/database.py` - it will insert all data from downloaded json db files to MySQL database with the structure above.

_Note: `nvdsearch/tools/config.json` file is used by python scripts to connect to MySQL server._
