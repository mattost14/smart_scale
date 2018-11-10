from minha_saude import db
from flask_security import Security, SQLAlchemyUserDatastore, \
    UserMixin, RoleMixin, login_required, current_user, logout_user
from datetime import datetime

# Define models
roles_users = db.Table(
    'roles_users',
    db.Column('user_id', db.Integer(), db.ForeignKey('user.id')),
    db.Column('role_id', db.Integer(), db.ForeignKey('role.id'))
)

class Role(db.Model, RoleMixin):
    id = db.Column(db.Integer(), primary_key=True)
    name = db.Column(db.String(80), unique=True)
    description = db.Column(db.String(255))

    def __str__(self):
        return self.name

class User(db.Model, UserMixin):
    id = db.Column(db.Integer, primary_key=True)
    first_name = db.Column(db.String(255))
    last_name = db.Column(db.String(255))
    email = db.Column(db.String(255), unique=True)
    password = db.Column(db.String(255))
    active = db.Column(db.Boolean())
    confirmed_at = db.Column(db.DateTime())
    profile_image = db.Column(db.String(20), nullable=False, default='default.jpg')
    roles = db.relationship('Role', secondary=roles_users,
                            backref=db.backref('users', lazy='dynamic'))
    measure = db.relationship('Measure',backref=db.backref('user', lazy=True))
    source = db.relationship('Source',backref=db.backref('user', lazy=True))

    def __str__(self):
        return self.email

class Measure(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    date = db.Column(db.DateTime,nullable=False, default=datetime.utcnow)
    weight = db.Column(db.Float, nullable=False)
    user_id = db.Column(db.Integer, db.ForeignKey('user.id'), nullable=False)

    def __init__(self, date, weight, id):
        self.date = date
        self.weight = weight
        self.user_id = id

    def __str__(self):
        return {"user": self.user, "weight (kg)": self.weight}
    def __repr__(self):
        return {"user": self.user, "weight (kg)": self.weight}


class Source (db.Model):
    id = db.Column(db.Integer, primary_key=True)
    puplic_id = db.Column(db.Integer, nullable=False)
    source_name = db.Column(db.String(20), nullable=False)
    data_type = db.Column(db.String(20))
    user_id = db.Column(db.Integer, db.ForeignKey('user.id'), nullable=False)

    def __init__(self, puplic_id, source_name,data_type, user_id):
        self.puplic_id = puplic_id
        self.source_name = source_name
        self.data_type = data_type
        self.user_id = user_id