#include "Controller.h"

namespace vm {

Controller::Controller(QObject* _parent) : QObject(_parent) {
	qRegisterMetaType<ApplicationError>("ApplicationError");
	qRegisterMetaType<size_t>("size_t");
	qRegisterMetaType<MachineRepresentation>("MachineRepresentation");
	qRegisterMetaType<RobotRepresentation>("RobotRepresentation");


	this->prefix = "";

	this->addConcurrentTaskQueue("A");
	this->addConcurrentTaskQueue("B");
	this->addConcurrentTaskQueue("C");

	this->machine_manager_thread = new QThread();
	this->machine_manager_thread->start();

	this->machine_manager = new MachineManager();
	this->machine_manager->moveToThread(this->machine_manager_thread);

	//this->plc = new VirtualPLC();
	//this->plc->initOperandList();

	//QObject::connect(this->plc, SIGNAL(operandsChanged(std::map<QString, int>)), this, SLOT(processOperandValuesChanged(std::map<QString, int>)));

	//QString ip = "127.0.0.1";
	//std::vector<int> ports;
	//ports.push_back(10001);
	//ports.push_back(10002);
	//ports.push_back(10003);
	//if (this->plc->startServers(ip, ports) == true) {
	//	qDebug() << "Servers running!";
	//} else {
	//	qDebug() << "Starting servers failed!";
	//}
}

Controller::~Controller() {
	for (std::map<QString, QWaitCondition*>::iterator it = this->concurrent_task_wait_condition.begin(); it != this->concurrent_task_wait_condition.end(); ++it) {
		if (it->second != NULL) {
			delete it->second;
			it->second = NULL;
		}
	}
	this->concurrent_task_wait_condition.clear();

	for (std::map<QString, QMutex*>::iterator it = this->concurrent_task_wait_condition_mutex.begin(); it != this->concurrent_task_wait_condition_mutex.end(); ++it) {
		if (it->second != NULL) {
			delete it->second;
			it->second = NULL;
		}
	}
	this->concurrent_task_wait_condition_mutex.clear();

	for (std::map<QString, QReadWriteLock*>::iterator it = this->concurrent_task_wait_condition_data_lock.begin(); it != this->concurrent_task_wait_condition_data_lock.end(); ++it) {
		if (it->second != NULL) {
			delete it->second;
			it->second = NULL;
		}
	}
	this->concurrent_task_wait_condition_data_lock.clear();

	delete this->machine_manager;
}

QString Controller::getPrefix() const {
	QReadLocker locker(&this->data_lock);
	return this->prefix;
}

void Controller::setPrefix(QString _prefix) {
	QWriteLocker locker(&this->data_lock);
	this->prefix = _prefix;
}

QString Controller::getMachineConfigurationBaseDirectoryPath() const {
	QString machine_configuration_directory_path = this->getPrefix();
	machine_configuration_directory_path += "/RoboWrapControl3";

	return machine_configuration_directory_path;
}

QString Controller::getMachineConfigurationDirectoryPath() const {
	QString machine_configuration_directory_path = this->getMachineConfigurationBaseDirectoryPath();
	machine_configuration_directory_path += "/machine_";
	machine_configuration_directory_path += this->getCurrentMachineName();

	return machine_configuration_directory_path;
}


QString Controller::getCurrentMachineName() const {
	QReadLocker locker(&this->data_lock);
	return this->current_machine_name;
}

void Controller::setCurrentMachineName(QString _current_machine_name) {
	QWriteLocker locker(&this->data_lock);
	this->current_machine_name = _current_machine_name;
}

QStringList Controller::retrieveAvailableMachineNames() const {
	QStringList machine_names;

	QStringList name_filters;
	name_filters.push_back("machine_*");

	QDir directory(this->getMachineConfigurationBaseDirectoryPath());
	QStringList entries = directory.entryList(name_filters, QDir::Dirs, QDir::Name);

	for (int i = 0; i< entries.size(); ++i) {
		QString machine_name = entries[i].section("_", -1, -1);
		if (machine_name == "default") {
			continue;
		}

		machine_names.push_back(machine_name);
	}

	return machine_names;
}

MachineManager* Controller::getMachineManager() {
	return this->machine_manager;
}

const MachineManager* Controller::getMachineManager() const {
	return this->machine_manager;
}


void Controller::enqueueConcurrentTask(QString _queue_name, Controller::ConcurrentTask::GenericCallback _function_callback, QString _result_slot_name) {
	std::vector<Controller::ConcurrentTask::GenericCallback> function_callbacks;
	function_callbacks.push_back(_function_callback);

	this->enqueueConcurrentTask(_queue_name, function_callbacks, _result_slot_name);
}

void Controller::enqueueConcurrentTask(QString _queue_name, std::vector<Controller::ConcurrentTask::GenericCallback> _function_callbacks, QString _result_slot_name) {
	std::map<QString, std::queue<ConcurrentTask*> >::iterator queue_it = this->concurrent_task_queues.find(_queue_name);
	if (queue_it == this->concurrent_task_queues.end()) {
		return;
	}

	std::map<QString, int>::iterator triggered_flag_it = this->concurrent_task_processing_triggerd_flag.find(_queue_name);
	if (triggered_flag_it == this->concurrent_task_processing_triggerd_flag.end()) {
		return;
	}

	if (_function_callbacks.size() == 0) {
		return;
	}

	Controller::ConcurrentTask* concurrent_task = new Controller::ConcurrentTask();
	concurrent_task->queue_name = _queue_name;

	for (size_t i = 0; i < _function_callbacks.size(); ++i) {
		concurrent_task->function_callbacks.push_back(_function_callbacks[i]);
	}

	for (size_t i = 0; i < concurrent_task->function_callbacks.size(); ++i) {
		QFutureWatcher<int>* future_watcher = new QFutureWatcher<int>();
		future_watcher->setProperty("QUEUE", QVariant(_queue_name));
		future_watcher->setProperty("INDEX", QVariant(i));
		QObject::connect(future_watcher, SIGNAL(finished()), this, SLOT(processConcurrentCommandFinished()));

		concurrent_task->future_watchers.push_back(future_watcher);
	}

	concurrent_task->result_slot_name = _result_slot_name;
	concurrent_task->status.init(concurrent_task->function_callbacks.size());

	bool trigger_command_processing_flag = false;
	this->concurrent_tasks_lock.lockForWrite();
	queue_it->second.push(concurrent_task);

	if (triggered_flag_it->second == 0) {
		triggered_flag_it->second = 1;
		trigger_command_processing_flag = true;
	}
	this->concurrent_tasks_lock.unlock();

	if (trigger_command_processing_flag == true) {
		this->triggerConcurrentComandProcessing(_queue_name);
	}
}


void Controller::processConcurrentCommandFinished() {
	QObject* sender = this->sender();
	if (sender == NULL) {
		return;
	}

	QFutureWatcher<int>* future_watcher = reinterpret_cast<QFutureWatcher<int>* >(sender);
	if (future_watcher == NULL) {
		return;
	}

	QString queue_name = future_watcher->property("QUEUE").toString();
	size_t index = future_watcher->property("INDEX").toUInt();

	std::map<QString, std::queue<ConcurrentTask*> >::iterator queue_it = this->concurrent_task_queues.find(queue_name);
	if (queue_it == this->concurrent_task_queues.end()) {
		return;
	}

	bool finished_flag = false;
	bool trigger_command_processing_flag = false;

	this->concurrent_tasks_lock.lockForWrite();

	queue_it->second.front()->finished_counter += 1;

	if (queue_it->second.front()->finished_counter == queue_it->second.front()->future_watchers.size()) {
		finished_flag = true;
	}

	int result = future_watcher->result();
	queue_it->second.front()->status.setStatusCode(index, result);

	if (finished_flag == true) {
		QString result_slot_name = queue_it->second.front()->result_slot_name;
		Controller::ConcurrentTask::Status status = queue_it->second.front()->status;

		delete queue_it->second.front();
		queue_it->second.front() = NULL;
		queue_it->second.pop();

		this->concurrent_tasks_lock.unlock();

		this->triggerConcurrentComandProcessing(queue_name);

		QMetaObject::invokeMethod(this, result_slot_name.toLocal8Bit().data(), Q_ARG(Controller::ConcurrentTask::Status, status));
	} else {
		this->concurrent_tasks_lock.unlock();
	}
}

void Controller::triggerConcurrentComandProcessing(QString _queue_name) {
	std::map<QString, int>::iterator triggered_flag_it = this->concurrent_task_processing_triggerd_flag.find(_queue_name);
	if (triggered_flag_it == this->concurrent_task_processing_triggerd_flag.end()) {
		return;
	}

	std::map<QString, std::queue<ConcurrentTask*> >::iterator queue_it = this->concurrent_task_queues.find(_queue_name);
	if (queue_it == this->concurrent_task_queues.end()) {
		return;
	}

	this->concurrent_tasks_lock.lockForWrite();

	if (queue_it->second.empty() == true) {
		triggered_flag_it->second = 0;

		this->concurrent_tasks_lock.unlock();

		return;
	}

	for (size_t i = 0; i < queue_it->second.front()->future_watchers.size(); ++i) {
		QFuture<int> future = QtConcurrent::run(queue_it->second.front()->function_callbacks[i]);
		queue_it->second.front()->future_watchers[i]->setFuture(future);
	}

	this->concurrent_tasks_lock.unlock();
}


void Controller::addConcurrentTaskQueue(QString _queue_name) {
	this->concurrent_task_processing_triggerd_flag[_queue_name] = 0;
	this->concurrent_task_queues[_queue_name] = std::queue<ConcurrentTask*>();
	this->concurrent_task_wait_condition_mutex[_queue_name] = new QMutex();
	this->concurrent_task_wait_condition[_queue_name] = new QWaitCondition();
	this->concurrent_task_wait_condition_data_lock[_queue_name] = new QReadWriteLock();
	this->concurrent_task_wait_condition_data[_queue_name] = ConcurrentTaskDataContainer();
}



void Controller::initializeApplication() {
	QString queue_name = "A";

	this->enqueueConcurrentTask(queue_name, boost::bind(&Controller::initializeApplicationConcurrent, this, queue_name), "processInitializeApplicationFinished");
}

int Controller::initializeApplicationConcurrent(QString _queue_name) {
	qDebug() << "Controller::initializeApplicationConcurrent()";
	
	QStringList machine_names = this->retrieveAvailableMachineNames();

	if (machine_names.size() == 0) {
		//QMessageBox::warning(this, tr("Machine Configuration"), tr("No machine configuration found!"));

		ApplicationError application_error;
		application_error.setLevel(ApplicationError::LEVEL_ERROR);
		application_error.setTitle(tr("Machine Configuration"));
		application_error.setMessage(tr("No machine configuration found!"));
		application_error.setImageCode(0);
		emit applicationErrorOccured(application_error);

		return 1;
	}
	else if (machine_names.size() == 1) {
		this->setCurrentMachineName(machine_names[0]);
	} else {
		std::map<QString, QMutex*>::iterator wc_lock_it = this->concurrent_task_wait_condition_mutex.find(_queue_name);
		if (wc_lock_it == this->concurrent_task_wait_condition_mutex.end() || wc_lock_it->second == NULL) {
			return 1;
		}

		std::map<QString, QWaitCondition*>::iterator wc_it = this->concurrent_task_wait_condition.find(_queue_name);
		if (wc_it == this->concurrent_task_wait_condition.end() || wc_it->second == NULL) {
			return 1;
		}

		emit requestSelectMachineDialog(_queue_name, machine_names);

		wc_lock_it->second->lock();
		wc_it->second->wait(wc_lock_it->second);
		wc_lock_it->second->unlock();


		std::map<QString, QReadWriteLock*>::iterator wc_data_lock_it = this->concurrent_task_wait_condition_data_lock.find(_queue_name);
		if (wc_data_lock_it == this->concurrent_task_wait_condition_data_lock.end() || wc_data_lock_it->second == NULL) {
			return 1;
		}

		std::map<QString, ConcurrentTaskDataContainer>::iterator wc_data_it = this->concurrent_task_wait_condition_data.find(_queue_name);
		if (wc_data_it == this->concurrent_task_wait_condition_data.end()) {
			return 1;
		}

		wc_data_lock_it->second->lockForRead();
		ConcurrentTaskDataContainer wait_condition_data = wc_data_it->second;
		wc_data_lock_it->second->unlock();

		QString answer = wait_condition_data.getValue("ANSWER").toString();

		if (answer != "YES") {
			ApplicationError application_error;
			application_error.setLevel(ApplicationError::LEVEL_ERROR);
			application_error.setTitle(tr("Machine ID"));
			application_error.setMessage(tr("No machine ID selected!"));
			application_error.setImageCode(0);
			emit applicationErrorOccured(application_error);

			return 1;
		}

		QString machine_name = wait_condition_data.getValue("MACHINE_NAME").toString();

		this->setCurrentMachineName(machine_name);
	}

	QString robot_configuration_path = this->getMachineConfigurationDirectoryPath();
	robot_configuration_path += QString::fromLatin1("/config/robot.xml");

	QString force_sensor_configuration_path = this->getMachineConfigurationDirectoryPath();
	force_sensor_configuration_path += QString::fromLatin1("/config/force_sensor.xml");

	QString machine_configuration_path = this->getMachineConfigurationDirectoryPath();
	machine_configuration_path += QString::fromLatin1("/config/machine_configuration.xml");

	// invoke: this->machine_manager->initialize(robot_configuration_path, machine_configuration_path)
	bool status = false;
	QMetaObject::invokeMethod(this->machine_manager, "initialize", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, status), Q_ARG(QString, robot_configuration_path), Q_ARG(QString, machine_configuration_path), Q_ARG(QString, force_sensor_configuration_path));
	if (status == false) {
		return 1;
	}

	qDebug() << "Controller::initializeApplicationConcurrent() DONE";

	return 0;
}

void Controller::processInitializeApplicationFinished(Controller::ConcurrentTask::Status _status) {
	if (_status.isSuccessful() == true) {
		emit initializeApplicationFinished(true);
	} else {
		emit initializeApplicationFinished(false);
	}
}


void Controller::uninitializeApplication() {
	QString queue_name = "A";

	this->enqueueConcurrentTask(queue_name, boost::bind(&Controller::uninitializeApplicationConcurrent, this, queue_name), "processUninitializeApplicationFinished");
}

int Controller::uninitializeApplicationConcurrent(QString _queue_name) {
	// invoke: this->machine_manager->uninitialize();
	bool status = false;
	QMetaObject::invokeMethod(this->machine_manager, "uninitialize", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, status));
	if (status == false) {
		return 1;
	}

	return 0;
}

void Controller::processUninitializeApplicationFinished(Controller::ConcurrentTask::Status _status) {
	if (_status.isSuccessful() == true) {
		emit uninitializeApplicationFinished(true);
	} else {
		emit uninitializeApplicationFinished(false);
	}
}


void Controller::enableAutomaticRead(size_t _line_id, size_t _robot_id) {
	QString queue_name = "A";

	this->enqueueConcurrentTask(queue_name, boost::bind(&Controller::enableAutomaticReadConcurrent, this, queue_name, _line_id, _robot_id), "processEnableAutomaticReadFinished");
}

int Controller::enableAutomaticReadConcurrent(QString _queue_name, size_t _line_id, size_t _robot_id) {
	// invoke: this->machine_manager->enableAutomaticRead(_line_id, _robot_id);
	bool status = false;
	QMetaObject::invokeMethod(this->machine_manager, "enableAutomaticRead", Qt::BlockingQueuedConnection, Q_RETURN_ARG(bool, status), Q_ARG(size_t, _line_id), Q_ARG(size_t, _robot_id));
	if (status == false) {
		return 1;
	}

	return 0;
}

void Controller:: processEnableAutomaticReadFinished(Controller::ConcurrentTask::Status _status) {
	if (_status.isSuccessful() == true) {
		emit enableAutomaticReadFinished(true);
	} else {
		emit enableAutomaticReadFinished(false);
	}
}


void Controller::disableAutomaticRead(size_t _line_id, size_t _robot_id) {
	QString queue_name = "A";

	this->enqueueConcurrentTask(queue_name, boost::bind(&Controller::disableAutomaticReadConcurrent, this, queue_name, _line_id, _robot_id), "processDisableAutomaticReadFinished");
}

