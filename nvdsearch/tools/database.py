import pymysql.cursors
import json
import os


class MySQLDatabase:
    def __init__(self):
        with open("config.json", encoding="utf8") as f:
            self.config = json.load(f)
        self.connection =\
            pymysql.connect(**self.config['mysql'],
                            charset='utf8mb4',
                            cursorclass=pymysql.cursors.DictCursor)
        if not self.connection.open:
            raise Exception("Couldn't open connection")
        self.cursor = self.connection.cursor()

    def __del__(self):
        try:
            self.connection.close()
        except pymysql.err.Error:
            pass

    def update_db(self, data):
        def to_mysql_date_format(str_datetime):
            return str_datetime.replace("T", " ").replace("Z", ":00")
        
        def insert(table_name, **kwargs):
            esc = lambda val: self.connection.escape(val)
            select_query = "SELECT id FROM `{table_name}` WHERE "\
                               .format(table_name=table_name) +\
                           " AND ".join("`{key}`={value}".format(key=key, value=esc(value))
                                        for key, value in kwargs.items())
            query_ids = self.cursor.execute(select_query)
            if query_ids:
                rows = [row for row in self.cursor]
                return rows[0]['id']
            insert_query = "INSERT INTO `{table_name}` ("\
                               .format(table_name=table_name) +\
                           ", ".join("`{key}`".format(key=key) for key in kwargs) +\
                           ") VALUES ({values})".format(values="%s, "*(len(kwargs)-1) + "%s")
            rows_inserted = self.cursor.execute(insert_query, tuple(kwargs.values()))
            query_ids = self.cursor.execute(select_query)
            if query_ids:
                self.connection.commit()
                rows = [row for row in self.cursor]
                return rows[0]['id']
            else:
                self.connection.rollback()
                
        cve_data = {}
        for cve_item in data['CVE_Items']:
            cve = cve_item['cve']
            cve_data['cve_name'] = cve['CVE_data_meta']['ID']
            cve_data['published_date'] = to_mysql_date_format(cve_item['publishedDate'])
            cve_data['last_modified_date'] = to_mysql_date_format(cve_item['lastModifiedDate'])
            
            cvssv = '3' if 'baseMetricV3' in cve_item['impact'] else '2'
            try:
                cve_data['base_score'] = cve_item['impact']['baseMetricV' + cvssv]['cvssV' + cvssv]['baseScore']
                cve_data['version'] = cvssv
            except KeyError:  # Skip CVEs with no impact
                continue

            # Insert data
            id_severity = insert("severity",
                                 base_score=cve_data['base_score'],
                                 version=cve_data['version'])
            
            id_cve = insert("cve",
                            cve_name=cve_data['cve_name'],
                            id_severity=id_severity,
                            published_date=cve_data['published_date'],
                            last_modified_date=cve_data['last_modified_date'])
            
            for description_data in cve['description']['description_data']:
                insert("description",
                       id_cve=id_cve,
                       lang=description_data['lang'],
                       value=description_data['value'])
                
            for reference_data in cve['references']['reference_data']:
                insert("reference",
                       id_cve=id_cve,
                       url=reference_data['url'])
            
            for vendor_data in cve['affects']['vendor']['vendor_data']:
                cve_data['vendor_name'] = vendor_data['vendor_name']
                
                id_vendor = insert("vendor",
                                   vendor_name=cve_data['vendor_name'])
                
                for product_data in vendor_data['product']['product_data']:
                    cve_data['product_name'] = product_data['product_name']
                    
                    id_product = insert("product",
                                        id_vendor=id_vendor,
                                        product_name=cve_data['product_name'])
                    
                    id_product_cve = insert("product_cve",
                                            id_product=id_product,
                                            id_cve=id_cve)
                    
                    for version_data in product_data['version']['version_data']:
                        cve_data['version_value'] = version_data['version_value']
                        
                        id_version = insert("version",
                                            id_product=id_product,
                                            version_value=cve_data['version_value'])
                        
                        insert("product_cve_version",
                               id_product_cve=id_product_cve,
                               id_version=id_version)


if __name__ == "__main__":
    mysql_db = MySQLDatabase()

    with open("config.json", encoding="utf8") as f:
        config = json.load(f)

    db_path = os.path.join(os.path.dirname(__file__), config['nvd_path'])
    root, _, files = next(os.walk(db_path))
    dbs = {}
    print('Processing years: ')
    for file in files:
        filepath = os.path.join(root, file)
        with open(filepath, encoding="utf8") as f:
            year = int(file.split('-')[-1].split('.')[0])
            print('  {}'.format(year))
            mysql_db.update_db(json.load(f))
    
    print('Done')