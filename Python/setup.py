from setuptools import setup

setup(
    name='headertool',
    version='1.0.4',
    entry_points={
        'console_scripts': [
            'headertool = headertool.core:main'
        ]
    },
    author='Brette',
    description='A CLI tool for headering ROMs',
    url='https://github.com/BrettefromNesUniverse/HeaderTool',
)