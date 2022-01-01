from distutils.core import setup
from setuptools import find_packages
import os

current_directory = os.path.dirname(os.path.abspath(__file__))
try:
    with open(os.path.join(current_directory, 'README.md'), encoding='utf-8') as f:
        long_description = f.read()
except Exception:
    long_description = ''

setup(
  name='DDWifiBridge',
  version='0.8.0',
  author='Trevor Lee',
  author_email='trev_lee@hotmail.com',
  description='DumbDisplay Wifi Bridge for DumbDisplay Arduino Lirary',
  long_description=long_description,
  long_description_content_type="text/markdown",
  url='https://github.com/trevorwslee/DDWifiBridge',
  license='MIT',
  packages=find_packages('.'),
  entry_points={
        'console_scripts': [
            'DDWifiBridge = DDWifiBridge.ddbridge:main',
        ],
  }
)
