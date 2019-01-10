#ifndef VM_ROBOT_H
#define VM_ROBOT_H

#include <QObject>
#include <QVariant>
#include <QString>
#include <QCoreApplication>
#include <QReadWriteLock>
#include <rl/math/Transform.h>
#include <rl/math/Unit.h>
#include <rl/mdl/Kinematic.h>
#include <rl/mdl/Model.h>
#include <rl/mdl/XmlFactory.h>
#include <rl/mdl/InverseKinematics.h>
#include <rl/mdl/NloptInverseKinematics.h>

#include <map>

#include "representations/RobotRepresentation.h"
#include "RobotJointAngles.h"
#include "RobotPose.h"


namespace vm {

class Robot : public QObject {
	Q_OBJECT
public:
	class Attribute {
	public:
		enum ValueType {
			TYPE_UNKNOWN,
			TYPE_ENUM,
			TYPE_INT,
			TYPE_REAL,
			TYPE_STRING
		};

		Attribute();
		void reset();

		void initValueAsInt(int _default_value, int _min_value, int _max_value, int _step_size);
		void initValueAsReal(double _default_value, double _min_value, double _max_value, double _step_size);
		void initValueAsString(const QString& _default_value);
		void initValueAsEnum(const QStringList& _value_list);

		QString getName() const;
		void setName(const QString& _name);

		bool isReadOnly() const;
		void setReadOnly(bool _read_only_flag);

		Attribute::ValueType getValueType() const;
		void setValueType(Attribute::ValueType _value_type);

		QVariant getValue() const;
		void setValue(const QVariant& _value);

		QVariant getDefaultValue() const;
		void setDefaultValue(const QVariant& _default_value);

		QVariant getMinValue() const;
		void setMinValue(const QVariant& _min_value);

		QVariant getMaxValue() const;
		void setMaxValue(const QVariant& _max_value);

		QVariant getStepSize() const;
		void setStepSize(const QVariant& _step_size);

	private:
		QString name;
		bool read_only_flag;

		Attribute::ValueType value_type;
		QVariant value;
		QVariant default_value;
		QVariant min_value;
		QVariant max_value;
		QVariant step_size;
	};

	typedef std::map<QString, Robot::Attribute> AttributeMap;

	typedef AttributeMap::iterator AttributeIter;
	typedef AttributeMap::const_iterator AttributeConstIter;


	Robot(QObject* _parent = NULL);
	virtual ~Robot();

	virtual QString getVendorName() const;
	virtual QString getProductName() const;

	virtual bool isAutomaticReadEnabled() const;
	virtual void setAutomaticReadEnabled(bool _automatic_read_enabled_flag);

	virtual size_t getNumberOfAttributes() const;
	virtual QStringList getAttributeNames() const;

	virtual bool hasAttribute(const QString& _name) const;
	virtual Robot::Attribute getAttribute(const QString& _name) const;
	virtual void setAttribute(Robot::Attribute _attribute);
	virtual void removeAttribute(const QString& _name);
	virtual void removeAllAttributes();

	virtual bool getAttributeValue(const QString& _name, Robot::Attribute::ValueType& _value_type, QVariant& _value) const;
	virtual bool setAttributeValue(const QString& _name, const QVariant& _value);


	virtual size_t getNumberOfConnectionAttributes(size_t _connection_index) const;
	virtual QStringList getConnectionAttributeNames(size_t _connection_index) const;

	virtual bool hasConnectionAttribute(size_t _connection_index, const QString& _name) const;
	virtual Robot::Attribute getConnectionAttribute(size_t _connection_index, const QString& _name) const;
	virtual void setConnectionAttribute(size_t _connection_index, Robot::Attribute _attribute);
	virtual void removeConnectionAttribute(size_t _connection_index, const QString& _name);
	virtual void removeAllConnectionAttributes(size_t _connection_index);

	virtual bool getConnectionAttributeValue(size_t _connection_index, const QString& _name, Robot::Attribute::ValueType& _value_type, QVariant& _value) const;
	virtual bool setConnectionAttributeValue(size_t _connection_index, const QString& _name, const QVariant& _value);

	virtual size_t getNumberOfConnections() const;

	virtual bool isConnectionUseFlag(size_t _connection_index) const;
	virtual void setConnectionUseFlag(size_t _connection_index, bool _use_flag);


	virtual void initRobotRepresentation(RobotRepresentation& _robot_representation);

	virtual bool getJointAnglesVariable(QString _program_name, QString _variable_name, unsigned int _variable_index, RobotJointAngles& _joint_angles_variable) const;
	virtual bool removeJointAnglesVariable(QString _program_name, QString _variable_name, unsigned int _variable_index);
	
	virtual bool getNumericVariable(QString _program_name, QString _variable_name, unsigned int _variable_index, double& _numeriuc_variable) const;
	virtual bool removeNumericVariable(QString _program_name, QString _variable_name, unsigned int _variable_index);

	virtual bool getPoseVariable(QString _program_name, QString _variable_name, unsigned int _variable_index, RobotPose& _pose_variable) const;
	virtual bool removePoseVariable(QString _program_name, QString _variable_name, unsigned int _variable_index);

	virtual bool getStringVariable(QString _program_name, QString _variable_name, unsigned int _variable_index, QString& _string_variable) const;
	virtual bool removeStringVariable(QString _program_name, QString _variable_name, unsigned int _variable_index);

	virtual bool getParameter(QString _parameter_name, std::vector<double>& _values) const;
	virtual bool removeParameter(QString _parameter_name);
	
	virtual double getBatteryPowerOnTime() const;
	virtual void setBatteryPowerOnTime(double _battery_power_on_time);

	virtual double getBatteryRemainingTime() const;
	virtual void setBatteryRemainingTime(double _battery_remaining_time);


public slots:
	virtual void moveObjectToMainThread();
	virtual void moveObjectToOtherThread(QThread* _thread);

	virtual bool initialize(size_t _line_id) = 0;
	virtual bool uninitialize() = 0;

	virtual bool restart() = 0;


	virtual void setNumericVariable(QString _program_name, QString _variable_name, unsigned int _variable_index, const double& _numeric_variable);
	virtual void setStringVariable(QString _program_name, QString _variable_name, unsigned int _variable_index, const QString& _string_variable);
	virtual void setPoseVariable(QString _program_name, QString _variable_name, unsigned int _variable_index, const RobotPose& _pose_variable);
	virtual void setJointAnglesVariable(QString _program_name, QString _variable_name, unsigned int _variable_index, const RobotJointAngles& _joint_angles_variable);
	virtual void setParameter(QString _parameter_name, const std::vector<double>& _values);


	//void receivedData(QByteArray _data);

signals:
	void robotDataChanged();
	
	void restartRequested();

protected:
	QString vendor_name;
	QString product_name;
	bool use_flag;

	bool automatic_read_enabled_flag;

	Robot::AttributeMap attributes;

	std::vector<char> connection_use_flags;
	std::vector<Robot::AttributeMap> connection_attributes;

	mutable QReadWriteLock data_lock;

		// program_name -> variable_name -> variable_index (0==no index) -> variable
		std::map<QString, std::map<QString, std::map<unsigned int, RobotJointAngles> > > joint_angles_variables;
		std::map<QString, std::map<QString, std::map<unsigned int, double> > > numeric_variables;
		std::map<QString, std::map<QString, std::map<unsigned int, RobotPose> > > pose_variables;
		std::map<QString, std::map<QString, std::map<unsigned int, QString> > > string_variables;

		// parameter_name -> parameter values
		std::map<QString, std::vector<double> > parameters;

		double battery_power_on_time;
		double battery_remaining_time;
private:

	

};

}

#endif /* VM_ROBOT_H */