#include "Robot.h"

namespace vm {

Robot::Attribute::Attribute() {
	this->reset();
}

void Robot::Attribute::reset() {
	this->value_type = Robot::Attribute::TYPE_UNKNOWN;
	this->default_value = QVariant();
	this->min_value = QVariant();
	this->max_value = QVariant();
	this->step_size = QVariant();
	this->read_only_flag = false;
}

void Robot::Attribute::initValueAsInt(int _default_value, int _min_value, int _max_value, int _step_size) {
	this->value_type = Robot::Attribute::TYPE_INT;
	this->default_value = _default_value;
	this->min_value = _min_value;
	this->max_value = _max_value;
	this->step_size = _step_size;

	this->setValue(this->default_value);
}

void Robot::Attribute::initValueAsReal(double _default_value, double _min_value, double _max_value, double _step_size) {
	this->value_type = Robot::Attribute::TYPE_REAL;
	this->default_value = _default_value;
	this->min_value = _min_value;
	this->max_value = _max_value;
	this->step_size = _step_size;

	this->setValue(this->default_value);
}

void Robot::Attribute::initValueAsString(const QString& _default_value) {
	this->value_type = Robot::Attribute::TYPE_STRING;
	this->default_value = _default_value;
	this->min_value = QVariant();
	this->max_value = QVariant();
	this->step_size = QVariant();

	this->setValue(this->default_value);
}

void Robot::Attribute::initValueAsEnum(const QStringList& _value_list) {
	this->value_type = Robot::Attribute::TYPE_ENUM;
	this->default_value = _value_list;
	this->min_value = QVariant();
	this->max_value = QVariant();
	this->step_size = QVariant();

	if (_value_list.size() > 0) {
		this->setValue(_value_list[0]);
	}
	else {
		this->setValue(QVariant());
	}
}

QString Robot::Attribute::getName() const {
	return this->name;
}

void Robot::Attribute::setName(const QString& _name) {
	this->name = _name;
}

bool Robot::Attribute::isReadOnly() const {
	return this->read_only_flag;
}

void Robot::Attribute::setReadOnly(bool _read_only_flag) {
	this->read_only_flag = _read_only_flag;
}

Robot::Attribute::ValueType Robot::Attribute::getValueType() const {
	return this->value_type;
}

void Robot::Attribute::setValueType(Robot::Attribute::ValueType _value_type) {
	this->value_type = _value_type;
}

QVariant Robot::Attribute::getValue() const {
	QVariant temp;

	switch (this->value_type) {
	case TYPE_ENUM: {
		QString current_value = this->value.toString();

		QStringList value_list = this->default_value.toStringList();
		if (value_list.contains(current_value) == true) {
			temp = this->value;
		}
		else {
			temp = QVariant("");
		}
		break;
	}
	case TYPE_INT: {
		int current_value = this->value.toInt();
		int min_value = this->min_value.toInt();
		int max_value = this->max_value.toInt();
		int step_size = this->step_size.toInt();

		current_value = std::min(std::max(min_value, current_value), max_value);
		if (step_size != 0) {
			//TODO correct value depending on current value of step_size
		}

		temp = QVariant(current_value);

		break;
	}
	case TYPE_REAL: {
		double current_value = this->value.toDouble();
		double min_value = this->min_value.toDouble();
		double max_value = this->max_value.toDouble();
		double step_size = this->step_size.toDouble();

		current_value = std::min(std::max(min_value, current_value), max_value);
		if (step_size != 0.0) {
			//TODO correct value depending on current value of step_size
		}

		temp = QVariant(current_value);
		break;
	}
	case TYPE_STRING: {
		temp = value;
		break;
	}
	}

	return temp;
}

void Robot::Attribute::setValue(const QVariant& _value) {
	this->value = _value;
}

QVariant Robot::Attribute::getDefaultValue() const {
	return this->default_value;
}

void Robot::Attribute::setDefaultValue(const QVariant& _default_value) {
	this->default_value = _default_value;
}

QVariant Robot::Attribute::getMinValue() const {
	return this->min_value;
}

void Robot::Attribute::setMinValue(const QVariant& _min_value) {
	this->min_value = min_value;
}

QVariant Robot::Attribute::getMaxValue() const {
	return this->max_value;
}

void Robot::Attribute::setMaxValue(const QVariant& _max_value) {
	this->max_value = _max_value;
}

QVariant Robot::Attribute::getStepSize() const {
	return this->step_size;
}

void Robot::Attribute::setStepSize(const QVariant& _step_size) {
	this->step_size = _step_size;
}


//---------------------------------------------------------------------------------------------------------------------------------


Robot::Robot(QObject* _parent) : QObject(_parent) {
	this->automatic_read_enabled_flag = true;
}

Robot::~Robot() {
}


QString Robot::getVendorName() const {
	return this->vendor_name;
}

QString Robot::getProductName() const {
	return this->product_name;
}

bool Robot::isAutomaticReadEnabled() const {
	return this->automatic_read_enabled_flag;
}

void Robot::setAutomaticReadEnabled(bool _automatic_read_enabled_flag) {
	this->automatic_read_enabled_flag = _automatic_read_enabled_flag;
}

size_t Robot::getNumberOfAttributes() const {
	return this->attributes.size();
}

QStringList Robot::getAttributeNames() const {
	QStringList names;

	for (Robot::AttributeConstIter it = this->attributes.begin(); it != this->attributes.end(); ++it) {
		names.push_back(it->first);
	}

	return names;
}

bool Robot::hasAttribute(const QString& _name) const {
	Robot::AttributeConstIter it = this->attributes.find(_name);
	return (it != this->attributes.end());
}

Robot::Attribute Robot::getAttribute(const QString& _name) const {
	Robot::AttributeConstIter it = this->attributes.find(_name);
	if (it != this->attributes.end()) {
		return it->second;
	} else {
		return Robot::Attribute();
	}
}

void Robot::setAttribute(Robot::Attribute _attribute) {
	this->attributes[_attribute.getName()] = _attribute;
}

void Robot::removeAttribute(const QString& _name) {
	Robot::AttributeIter it = this->attributes.find(_name);

	if (it != this->attributes.end()) {
		this->attributes.erase(it);
	}
}

void Robot::removeAllAttributes() {
	this->attributes.clear();
}


bool Robot::getAttributeValue(const QString& _name, Robot::Attribute::ValueType& _value_type, QVariant& _value) const {
	if (this->hasAttribute(_name) == false) {
		return false;
	}

	Robot::Attribute attribute = this->getAttribute(_name);
	_value_type = attribute.getValueType();
	_value = attribute.getValue();

	return true;
}

bool Robot::setAttributeValue(const QString& _name, const QVariant& _value) {
	if (this->hasAttribute(_name) == false) {
		return false;
	}

	Robot::Attribute attribute = this->getAttribute(_name);
	switch (attribute.getValueType()) {
	case Robot::Attribute::TYPE_ENUM:
		if (_value.type() == QVariant::String) {
			attribute.setValue(_value);
		} else {
			return false;
		}
		break;
	case Robot::Attribute::TYPE_INT:
		if (_value.type() == QVariant::Int) {
			attribute.setValue(_value);
		} else {
			return false;
		}
		break;
	case Robot::Attribute::TYPE_REAL:
		if (_value.type() == QVariant::Double) {
			attribute.setValue(_value);
		} else {
			return false;
		}
		break;
	case Robot::Attribute::TYPE_STRING:
		if (_value.type() == QVariant::String) {
			attribute.setValue(_value);
		} else {
			return false;
		}
		break;
	}

	this->setAttribute(attribute);
	return true;
}


size_t Robot::getNumberOfConnectionAttributes(size_t _connection_index) const {
	if (_connection_index >= this->getNumberOfConnections()) {
		return 0;
	}

	return this->connection_attributes[_connection_index].size();
}

QStringList Robot::getConnectionAttributeNames(size_t _connection_index) const {
	if (_connection_index >= this->getNumberOfConnections()) {
		return QStringList();
	}

	QStringList names;

	for (Robot::AttributeConstIter it = this->connection_attributes[_connection_index].begin(); it != this->connection_attributes[_connection_index].end(); ++it) {
		names.push_back(it->first);
	}

	return names;
}


bool Robot::hasConnectionAttribute(size_t _connection_index, const QString& _name) const {
	if (_connection_index >= this->getNumberOfConnections()) {
		return false;
	}

	Robot::AttributeConstIter it = this->connection_attributes[_connection_index].find(_name);
	return (it != this->connection_attributes[_connection_index].end());
}

Robot::Attribute Robot::getConnectionAttribute(size_t _connection_index, const QString& _name) const {
	if (_connection_index >= this->getNumberOfConnections()) {
		return Attribute();
	}

	Robot::AttributeConstIter it = this->connection_attributes[_connection_index].find(_name);
	if (it != this->connection_attributes[_connection_index].end()) {
		return it->second;
	} else {
		return Robot::Attribute();
	}
}

void Robot::setConnectionAttribute(size_t _connection_index, Robot::Attribute _attribute) {
	if (_connection_index >= this->getNumberOfConnections()) {
		return;
	}

	this->connection_attributes[_connection_index][_attribute.getName()] = _attribute;
}

void Robot::removeConnectionAttribute(size_t _connection_index, const QString& _name) {
	if (_connection_index >= this->getNumberOfConnections()) {
		return;
	}

	Robot::AttributeIter it = this->connection_attributes[_connection_index].find(_name);
	if (it != this->connection_attributes[_connection_index].end()) {
		this->connection_attributes[_connection_index].erase(it);
	}
}

void Robot::removeAllConnectionAttributes(size_t _connection_index) {
	if (_connection_index >= this->getNumberOfConnections()) {
		return;
	}

	this->connection_attributes[_connection_index].clear();
}

bool Robot::getConnectionAttributeValue(size_t _connection_index, const QString& _name, Robot::Attribute::ValueType& _value_type, QVariant& _value) const {
	if (_connection_index >= this->getNumberOfConnections()) {
		return false;
	}

	if (this->hasConnectionAttribute(_connection_index, _name) == false) {
		return false;
	}

	Robot::Attribute attribute = this->getConnectionAttribute(_connection_index, _name);
	_value_type = attribute.getValueType();
	_value = attribute.getValue();

	return true;
}

bool Robot::setConnectionAttributeValue(size_t _connection_index, const QString& _name, const QVariant& _value) {
	if (_connection_index >= this->getNumberOfConnections()) {
		return false;
	}

	if (this->hasConnectionAttribute(_connection_index, _name) == false) {
		return false;
	}

	Robot::Attribute attribute = this->getConnectionAttribute(_connection_index, _name);
	switch (attribute.getValueType()) {
	case Robot::Attribute::TYPE_ENUM:
		if (_value.type() == QVariant::String) {
			attribute.setValue(_value);
		} else {
			return false;
		}
		break;
	case Robot::Attribute::TYPE_INT:
		if (_value.type() == QVariant::Int) {
			attribute.setValue(_value);
		} else {
			return false;
		}
		break;
	case Robot::Attribute::TYPE_REAL:
		if (_value.type() == QVariant::Double) {
			attribute.setValue(_value);
		} else {
			return false;
		}
		break;
	case Robot::Attribute::TYPE_STRING:
		if (_value.type() == QVariant::String) {
			attribute.setValue(_value);
		} else {
			return false;
		}
		break;
	}

	this->setConnectionAttribute(_connection_index, attribute);
	return true;
}

size_t Robot::getNumberOfConnections() const {
	return this->connection_attributes.size();
}


bool Robot::isConnectionUseFlag(size_t _connection_index) const {
	if (_connection_index >= this->connection_use_flags.size()) {
		return false;
	}

	return this->connection_use_flags[_connection_index] != 0;
}

void Robot::setConnectionUseFlag(size_t _connection_index, bool _use_flag) {
	if (_connection_index >= this->connection_use_flags.size()) {
		return;
	}

	if (_use_flag == true) {
		this->connection_use_flags[_connection_index] = 1;
	} else {
		this->connection_use_flags[_connection_index] = 0;
	}
}

void Robot::initRobotRepresentation(RobotRepresentation& _robot_representation) {
	_robot_representation.reset();

	for (std::map<QString, std::map<QString, std::map<unsigned int, QString> > >::const_iterator it0 = this->string_variables.begin(); it0 != this->string_variables.end(); ++it0) {
		for (std::map<QString, std::map<unsigned int, QString> >::const_iterator it1 = it0->second.begin(); it1 != it0->second.end(); ++it1) {
			for (std::map<unsigned int, QString>::const_iterator it2 = it1->second.begin(); it2 != it1->second.end(); ++it2) {
				StringVariableRepresentation variable_representation;
				variable_representation.setProgramName(it0->first);
				variable_representation.setName(it1->first);
				variable_representation.setIndex(it2->first);
				variable_representation.setValue(it2->second);

				_robot_representation.setStringVariableRepresentation(it0->first, it1->first, it2->first, variable_representation);
			}
		}
	}

	for (std::map<QString, std::map<QString, std::map<unsigned int, RobotJointAngles> > >::const_iterator it0 = this->joint_angles_variables.begin(); it0 != this->joint_angles_variables.end(); ++it0) {
		for (std::map<QString, std::map<unsigned int, RobotJointAngles> >::const_iterator it1 = it0->second.begin(); it1 != it0->second.end(); ++it1) {
			for (std::map<unsigned int, RobotJointAngles>::const_iterator it2 = it1->second.begin(); it2 != it1->second.end(); ++it2) {
				JointAnglesVariableRepresentation variable_representation;
				variable_representation.setProgramName(it0->first);
				variable_representation.setName(it1->first);
				variable_representation.setIndex(it2->first);
				variable_representation.setValue(
					it2->second.getJ1(),
					it2->second.getJ2(),
					it2->second.getJ3(),
					it2->second.getJ4(),
					it2->second.getJ5(),
					it2->second.getJ6(),
					it2->second.getJ7(),
					it2->second.getJ8()
				);

				_robot_representation.setJointAnglesVariableRepresentation(it0->first, it1->first, it2->first, variable_representation);
			}
		}
	}

	for (std::map<QString, std::map<QString, std::map<unsigned int, double> > >::const_iterator it0 = this->numeric_variables.begin(); it0 != this->numeric_variables.end(); ++it0) {
		for (std::map<QString, std::map<unsigned int, double> >::const_iterator it1 = it0->second.begin(); it1 != it0->second.end(); ++it1) {
			for (std::map<unsigned int, double>::const_iterator it2 = it1->second.begin(); it2 != it1->second.end(); ++it2) {
				NumericVariableRepresentation variable_representation;
				variable_representation.setProgramName(it0->first);
				variable_representation.setName(it1->first);
				variable_representation.setIndex(it2->first);
				variable_representation.setValue(it2->second);

				_robot_representation.setNumericVariableRepresentation(it0->first, it1->first, it2->first, variable_representation);
			}
		}
	}

	for (std::map<QString, std::map<QString, std::map<unsigned int, RobotPose> > >::const_iterator it0 = this->pose_variables.begin(); it0 != this->pose_variables.end(); ++it0) {
		for (std::map<QString, std::map<unsigned int, RobotPose> >::const_iterator it1 = it0->second.begin(); it1 != it0->second.end(); ++it1) {
			for (std::map<unsigned int, RobotPose>::const_iterator it2 = it1->second.begin(); it2 != it1->second.end(); ++it2) {
				PoseVariableRepresentation variable_representation;
				variable_representation.setProgramName(it0->first);
				variable_representation.setName(it1->first);
				variable_representation.setIndex(it2->first);
				variable_representation.setValue(
					it2->second.getX(),
					it2->second.getY(),
					it2->second.getZ(),
					it2->second.getA(),
					it2->second.getB(),
					it2->second.getZ(),
					it2->second.getF1(),
					it2->second.getF2()
					);

				_robot_representation.setPoseVariableRepresentation(it0->first, it1->first, it2->first, variable_representation);
			}
		}
	}

	for (std::map<QString, std::vector<double> >::const_iterator it = this->parameters.begin(); it != this->parameters.end(); ++it) {
		ParameterRepresentation parameter_representation;
		parameter_representation.setName(it->first);
		parameter_representation.setValues(it->second);

		_robot_representation.setParameterRepresentations(it->first, parameter_representation);
	}
}


bool Robot::getJointAnglesVariable(QString _program_name, QString _variable_name, unsigned int _variable_index, RobotJointAngles& _joint_angles_variable) const {
	QReadLocker locker(&this->data_lock);

	std::map<QString, std::map<QString, std::map<unsigned int, RobotJointAngles> > >::const_iterator it0 = this->joint_angles_variables.find(_program_name);
	if (it0 == this->joint_angles_variables.end()) {
		return false;
	}

	std::map<QString, std::map<unsigned int, RobotJointAngles> >::const_iterator it1 = it0->second.find(_variable_name);
	if (it1 == it0->second.end()) {
		return false;
	}

	std::map<unsigned int, RobotJointAngles>::const_iterator it2 = it1->second.find(_variable_index);
	if (it2 == it1->second.end()) {
		return false;
	}

	_joint_angles_variable = it2->second;
	return true;

}

bool Robot::removeJointAnglesVariable(QString _program_name, QString _variable_name, unsigned int _variable_index) {
	QWriteLocker locker(&this->data_lock);

	std::map<QString, std::map<QString, std::map<unsigned int, RobotJointAngles> > >::iterator it0 = this->joint_angles_variables.find(_program_name);
	if (it0 == this->joint_angles_variables.end()) {
		return false;
	}

	std::map<QString, std::map<unsigned int, RobotJointAngles> >::iterator it1 = it0->second.find(_variable_name);
	if (it1 == it0->second.end()) {
		return false;
	}

	std::map<unsigned int, RobotJointAngles>::iterator it2 = it1->second.find(_variable_index);
	if (it2 == it1->second.end()) {
		return false;
	}

	it1->second.erase(it2);
	return true;
}

void Robot::setJointAnglesVariable(QString _program_name, QString _variable_name, unsigned int _variable_index, const RobotJointAngles& _joint_angles_variable) {
	QWriteLocker locker(&this->data_lock);

	this->joint_angles_variables[_program_name][_variable_name][_variable_index] = _joint_angles_variable;

	locker.unlock();
	emit robotDataChanged();
}


bool Robot::getNumericVariable(QString _program_name, QString _variable_name, unsigned int _variable_index, double& _numeric_variable) const {
	QReadLocker locker(&this->data_lock);

	std::map<QString, std::map<QString, std::map<unsigned int, double> > >::const_iterator it0 = this->numeric_variables.find(_program_name);
	if (it0 == this->numeric_variables.end()) {
		return false;
	}

	std::map<QString, std::map<unsigned int, double> >::const_iterator it1 = it0->second.find(_variable_name);
	if (it1 == it0->second.end()) {
		return false;
	}

	std::map<unsigned int, double>::const_iterator it2 = it1->second.find(_variable_index);
	if (it2 == it1->second.end()) {
		return false;
	}

	_numeric_variable = it2->second;
	return true;
}

bool Robot::removeNumericVariable(QString _program_name, QString _variable_name, unsigned int _variable_index) {
	QWriteLocker locker(&this->data_lock);

	std::map<QString, std::map<QString, std::map<unsigned int, double> > >::iterator it0 = this->numeric_variables.find(_program_name);
	if (it0 == this->numeric_variables.end()) {
		return false;
	}

	std::map<QString, std::map<unsigned int, double> >::iterator it1 = it0->second.find(_variable_name);
	if (it1 == it0->second.end()) {
		return false;
	}

	std::map<unsigned int, double>::iterator it2 = it1->second.find(_variable_index);
	if (it2 == it1->second.end()) {
		return false;
	}

	it1->second.erase(it2);
	return true;
}

void Robot::setNumericVariable(QString _program_name, QString _variable_name, unsigned int _variable_index, const double& _numeric_variable) {
	bool variable_found_flag = false;
	bool variable_changed_flag = false;

	QWriteLocker locker(&this->data_lock);

	std::map<QString, std::map<QString, std::map<unsigned int, double> > >::iterator it0 = this->numeric_variables.find(_program_name);
	if (it0 != this->numeric_variables.end()) {
		std::map<QString, std::map<unsigned int, double> >::iterator it1 = it0->second.find(_variable_name);
		if (it1 != it0->second.end()) {
			std::map<unsigned int, double>::iterator it2 = it1->second.find(_variable_index);
			if (it2 != it1->second.end()) {
				variable_found_flag = true;
				if (it2->second != _numeric_variable) {
					it2->second = _numeric_variable;
					variable_changed_flag = true;
				}

			}
		}
	}

	if (variable_found_flag == false) {
		this->numeric_variables[_program_name][_variable_name][_variable_index] = _numeric_variable;
		variable_changed_flag = true;
	}

	locker.unlock();

	if (variable_changed_flag == true) {
		emit robotDataChanged();	
	}
}


bool Robot::getPoseVariable(QString _program_name, QString _variable_name, unsigned int _variable_index, RobotPose& _pose_variable) const {
	QReadLocker locker(&this->data_lock);

	std::map<QString, std::map<QString, std::map<unsigned int, RobotPose> > >::const_iterator it0 = this->pose_variables.find(_program_name);
	if (it0 == this->pose_variables.end()) {
		return false;
	}

	std::map<QString, std::map<unsigned int, RobotPose> >::const_iterator it1 = it0->second.find(_variable_name);
	if (it1 == it0->second.end()) {
		return false;
	}

	std::map<unsigned int, RobotPose>::const_iterator it2 = it1->second.find(_variable_index);
	if (it2 == it1->second.end()) {
		return false;
	}

	_pose_variable = it2->second;
	return true;
}

bool Robot::removePoseVariable(QString _program_name, QString _variable_name, unsigned int _variable_index) {
	QWriteLocker locker(&this->data_lock);

	std::map<QString, std::map<QString, std::map<unsigned int, RobotPose> > >::iterator it0 = this->pose_variables.find(_program_name);
	if (it0 == this->pose_variables.end()) {
		return false;
	}

	std::map<QString, std::map<unsigned int, RobotPose> >::iterator it1 = it0->second.find(_variable_name);
	if (it1 == it0->second.end()) {
		return false;
	}

	std::map<unsigned int, RobotPose>::iterator it2 = it1->second.find(_variable_index);
	if (it2 == it1->second.end()) {
		return false;
	}

	it1->second.erase(it2);
	return true;
}

void Robot::setPoseVariable(QString _program_name, QString _variable_name, unsigned int _variable_index, const RobotPose& _pose_variable) {
	QWriteLocker locker(&this->data_lock);

	this->pose_variables[_program_name][_variable_name][_variable_index] = _pose_variable;

	locker.unlock();
	emit robotDataChanged();
}


bool Robot::getStringVariable(QString _program_name, QString _variable_name, unsigned int _variable_index, QString& _string_variable) const {
	QReadLocker locker(&this->data_lock);

	std::map<QString, std::map<QString, std::map<unsigned int, QString> > >::const_iterator it0 = this->string_variables.find(_program_name);
	if (it0 == this->string_variables.end()) {
		return false;
	}

	std::map<QString, std::map<unsigned int, QString> >::const_iterator it1 = it0->second.find(_variable_name);
	if (it1 == it0->second.end()) {
		return false;
	}

	std::map<unsigned int, QString>::const_iterator it2 = it1->second.find(_variable_index);
	if (it2 == it1->second.end()) {
		return false;
	}

	_string_variable = it2->second;
	return true;
}

bool Robot::removeStringVariable(QString _program_name, QString _variable_name, unsigned int _variable_index) {
	QWriteLocker locker(&this->data_lock);

	std::map<QString, std::map<QString, std::map<unsigned int, QString> > >::iterator it0 = this->string_variables.find(_program_name);
	if (it0 == this->string_variables.end()) {
		return false;
	}

	std::map<QString, std::map<unsigned int, QString> >::iterator it1 = it0->second.find(_variable_name);
	if (it1 == it0->second.end()) {
		return false;
	}

	std::map<unsigned int, QString>::const_iterator it2 = it1->second.find(_variable_index);
	if (it2 == it1->second.end()) {
		return false;
	}

	it1->second.erase(it2);
	return true;
}

void Robot::setStringVariable(QString _program_name, QString _variable_name, unsigned int _variable_index, const QString& _string_variable) {
	QWriteLocker locker(&this->data_lock);

	this->string_variables[_program_name][_variable_name][_variable_index] = _string_variable;

	locker.unlock();
	emit robotDataChanged();
}

bool Robot::getParameter(QString _parameter_name, std::vector<double>& _values) const {
	QReadLocker locker(&this->data_lock);

	std::map<QString, std::vector<double> >::const_iterator it = this->parameters.find(_parameter_name);
	if (it == this->parameters.end()) {
		return false;
	}

	_values = it->second;
	return true;
}

bool Robot::removeParameter(QString _parameter_name) {
	QWriteLocker locker(&this->data_lock);

	std::map<QString, std::vector<double> >::iterator it = this->parameters.find(_parameter_name);
	if (it == this->parameters.end()) {
		return false;
	}

	this->parameters.erase(it);

	return true;
}

void Robot::setParameter(QString _parameter_name, const std::vector<double>& _values) {
	QWriteLocker locker(&this->data_lock);

	this->parameters[_parameter_name] = _values;

	locker.unlock();
	emit robotDataChanged();
}

double Robot::getBatteryPowerOnTime() const {
	QReadLocker locker(&this->data_lock);

	return this->battery_power_on_time;
}

void Robot::setBatteryPowerOnTime(double _battery_power_on_time) {
	QWriteLocker locker(&this->data_lock);

	this->battery_power_on_time = _battery_power_on_time;

	locker.unlock();
	emit robotDataChanged();
}

double Robot::getBatteryRemainingTime() const {
	QReadLocker locker(&this->data_lock);

	return this->battery_remaining_time;
}

void Robot::setBatteryRemainingTime(double _battery_remaining_time) {
	QWriteLocker locker(&this->data_lock);

	this->battery_remaining_time = _battery_remaining_time;

	locker.unlock();
	emit robotDataChanged();
}

void Robot::moveObjectToMainThread() {
	this->moveToThread(QCoreApplication::instance()->thread());
}

void Robot::moveObjectToOtherThread(QThread* _thread) {
	this->moveToThread(_thread);
}

}