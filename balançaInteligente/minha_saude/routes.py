
from flask_security import Security, SQLAlchemyUserDatastore, \
    login_required, current_user
from minha_saude.models import User, Role
from minha_saude import app, db, api, Resource
from flask import render_template, request, redirect, url_for, jsonify
from flask_security.forms import RegisterForm, ConfirmRegisterForm
from wtforms import StringField
from wtforms.validators import DataRequired
from minha_saude.models import Measure, Source
from datetime import datetime, timedelta

class ExtendedRegisterForm(RegisterForm):
    first_name = StringField('First Name', [DataRequired()])
    last_name = StringField('Last Name', [DataRequired()])

# Setup Flask-Security
user_datastore = SQLAlchemyUserDatastore(db, User, Role)
security = Security(app, user_datastore, register_form = ExtendedRegisterForm)

# Routes
@app.route('/')
@login_required
def home():
    labels = ["D","S","T","Q","Q","S","S"]
    today = datetime.now()
    lastsunday = today - timedelta((today.weekday()+1)%7)
    lastsunday.replace(hour=0, minute=0, second=0)
    nextsunday = today + timedelta(7-(today.weekday()+1)%7)
    nextsunday.replace(hour=0, minute=0, second=0)
    weektotalsec = (nextsunday-lastsunday).total_seconds()
    #data = Measure.query.filter(Measure.date.between(today-timedelta(days=7),today)).all()
    data = Measure.query.order_by(Measure.date.desc())
    return render_template('peso.html', labels=labels, data=data, lasttick = nextsunday, firsttick= lastsunday)

@app.route('/peso/add', methods =['POST'])
@login_required
def addData():
    date_input = request.form['date_input']
    if(str(date_input)==""):
        date_input=datetime.now()
    newData = Measure(datetime.strptime(date_input, '%d/%m/%y %H:%M'), request.form['data_input'], current_user.id)
    db.session.add(newData)
    db.session.commit()
    return redirect(url_for('home'))

@app.route('/deleterow/', methods=['POST'])
@login_required
def delete():
    id=request.args.get('id')
    if id != '':
        id=map(int, id.split(','))
        for id in id:
            row = Measure.query.get_or_404(id)
            db.session.delete(row)
            db.session.commit()
    return redirect(url_for('home'))

# Api setup
class Weight(Resource):

    """
    curl -H "Content-Type: application/json" -d '{"source_public_id":"1234", "measure":"76.6"}' http://0.0.0.0:5000/uploadmyweight
    """

    def post(self):
        data = request.get_json()
        user=Source.query.filter(Source.puplic_id==int(data["source_public_id"])).first()
        if not user:
            return jsonify({'Message': 'No user found for this source public id.'})
        user_id = user.user_id
        newMeasure = Measure(datetime.now(), float(data["measure"]), user_id)
        db.session.add(newMeasure)
        db.session.commit()
        return jsonify({'Message': 'Upload succeed.'})

api.add_resource(Weight, '/uploadmyweight')