import logging; module_logger = logging.getLogger(__name__)
from pathlib import Path
import subprocess, tempfile, shutil, datetime
from contextlib import contextmanager

# ----------------------------------------------------------------------

@contextmanager
def timeit(name):
    start = datetime.datetime.utcnow()
    try:
        yield
    finally:
        module_logger.info('{} <{}>'.format(name, datetime.datetime.utcnow() - start))

# ----------------------------------------------------------------------

@contextmanager
def tempdir():
    dir = tempfile.mkdtemp()
    try:
        yield Path(dir)
    finally:
        module_logger.info('removing {}'.format(dir))
        shutil.rmtree(dir, ignore_errors=True)

# ----------------------------------------------------------------------

sAcmacsSuffixes = {".acd1", ".acp1", ".acmacs-txt"}
def get_ace(temp_dir :Path, source1 :Path):
    if set(source1.suffixes) & sAcmacsSuffixes:
        source2 = temp_dir.joinpath(source1.stem + ".ace")
        subprocess.run("env ACMACS_ROOT=$HOME/ac/acmacs PYTHONPATH=$HOME/ac/acmacs:$HOME/ac/acmacs/build $HOME/c2r/bin/python3 $HOME/ac/acmacs/bin/convert.py -q '{}' '{}'".format(source1, source2), shell=True, check=True)
    elif source1.suffixes[-1] == ".ace":
        source2 = source1
    else:
        source2 = None
        module_logger.warning('Unsupported suffix in {}'.format(source1))
    return source2

# ----------------------------------------------------------------------

def get_ace_data(source :Path):
    if set(source.suffixes) & sAcmacsSuffixes:
        data = subprocess.run("env ACMACS_ROOT=$HOME/ac/acmacs PYTHONPATH=$HOME/ac/acmacs:$HOME/ac/acmacs/build $HOME/c2r/bin/python3 $HOME/ac/acmacs/bin/convert.py -q -f ace '{}' -".format(source), shell=True, stdout=subprocess.PIPE, check=True).stdout
    elif source.suffixes[-1] == ".ace":
        data = open(source, "rb").read()
    else:
        data = None
        module_logger.warning('Unsupported suffix in {}'.format(source1))
    return data

# ----------------------------------------------------------------------
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
