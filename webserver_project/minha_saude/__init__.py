from flask import Flask
from flask_sqlalchemy import SQLAlchemy
from flask_restful import Resource, Api


# Create Flask application
app = Flask(__name__)
app.config.from_pyfile('config.py')
api = Api(app)
db = SQLAlchemy(app)

from minha_saude import routes
