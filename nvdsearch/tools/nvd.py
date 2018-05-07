import os
import datetime
import hashlib
import requests
import logging
import json
import io
import zipfile
from multiprocessing.pool import ThreadPool


def get_db_address(year, type='json.zip'):
    return 'https://static.nvd.nist.gov/feeds/json/cve/1.0/nvdcve-1.0-{y}.{ext}'\
        .format(y=year, ext=type)


def sha256_checksum(filename, block_size=65536):
    sha256 = hashlib.sha256()
    with open(filename, 'rb') as f:
        for block in iter(lambda: f.read(block_size), b''):
            sha256.update(block)
    return sha256.hexdigest().upper()


class Downloader:
    def __init__(self, db_path=None):
        self.first_year = 2002  # hardcoded first year on NVD
        self.default_chunk_size = 65536
        with open("config.json", encoding="utf8") as f:
            self.config = json.load(f)
        
        self.db_path = os.path.join(os.path.abspath(__file__), self.config['nvd_path'])
        if db_path:
            self.db_path = db_path
            
    def _get_local_hashes(self):
        hashes = {}
        root, _, files = next(os.walk(self.db_path, topdown=False))
        for file in files:
            path = os.path.join(root, file)
            hashes[sha256_checksum(path)] = path
        return hashes
    
    def _get_online_hashes(self):
        cur_year = datetime.datetime.now().year
        hashes = {}
        # enclosed
        def get_online_hash(year):
            r = requests.get(get_db_address(year, 'meta'))
            sha256hash = r.text.split()[-1].split(':')[-1].upper()
            hashes[sha256hash] = get_db_address(year)
        try:
            p = ThreadPool(20)
            p.imap_unordered(get_online_hash, range(self.first_year, cur_year + 1))
            p.close()
            p.join()
        except requests.exceptions.RequestException as e:
            logging.error(str(e))
            return {}  # couldn't retrieve all hashes
        return hashes
    
    def _download_db_file(self, link):
        r = requests.get(link, stream=True)
        f = io.BytesIO()
        # saves data into memory file object as it arrives
        for chunk in r.iter_content(self.default_chunk_size):
            f.write(chunk)
        r.close()
        with zipfile.ZipFile(f, compression=zipfile.ZIP_DEFLATED) as resp_zip:
            resp_zip.extractall(self.db_path)
        f.close()

    def _db_dir_check_existence(self):
        os.makedirs(self.db_path, exist_ok=True)

    def update(self):
        self._db_dir_check_existence()
        
        new_hashes = self._get_online_hashes()
        old_hashes = self._get_local_hashes()

        old_to_delete = set(old_hashes) - set(new_hashes)
        for old_hash in old_to_delete:
            os.remove(old_hashes[old_hash])
            
        new_to_download = set(new_hashes) - set(old_hashes)
        # for new_hash in new_to_download:
        #     self._download_db_file(new_hashes[new_hash])
        p = ThreadPool(20)
        p.imap_unordered(self._download_db_file, (new_hashes[new_hash] for new_hash in new_to_download))
        p.close()
        p.join()
    
class DataRefactor:
    def __init__(self, db_path=None):
        self.dbs = {}
        self.db_path = os.path.join(os.path.dirname(__file__), 'db')
        if db_path:
            self.db_path = db_path
    
    def load(self):
        root, _, files = next(os.walk(self.db_path))
        for file in files:
            filepath = os.path.join(root, file)
            with open(filepath, encoding="utf8") as f:
                year = int(file.split('-')[-1].split('.')[0])
                self.dbs[year] = json.load(f)
        
        
if __name__ == "__main__":
    with open("config.json", encoding="utf8") as f:
        config = json.load(f)
    
    downloader = Downloader(config['nvd_path'])
    downloader.update()
    db = DataRefactor()
    db.load()
    
    print("Done.")