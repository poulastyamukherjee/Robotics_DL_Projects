#ifndef VM_CONTROLLER_H
#define VM_CONTROLLER_H

#include <QObject>
#include <QMetaObject>
#include <QString>
#include <QStringList>

#include <QReadWriteLock>


#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtConcurrent>
#endif

#include <QFutureWatcher>
#include <QMutex>
#include <QWaitCondition>

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <map>
#include <queue>

#include "ApplicationError.h"
#include "MachineManager.h"
#include "representations\MachineRepresentation.h"

namespace vm {

class Controller : public QObject {
	Q_OBJECT
public:
	struct ConcurrentTask {
		struct Status {
			void reset() {
				this->status_codes.clear();
			}

			void init(size_t _number) {
				this->reset();
				this->status_codes.resize(_number, 0);
			}

			size_t getNuberOfStatusCodes() const {
				return this->status_codes.size();
			}

			int getStatusCode(size_t _index) const {
				return this->status_codes[_index];
			}

			void setStatusCode(size_t _index, int _status_code) {
				this->status_codes[_index] = _status_code;
			}

			bool isSuccessful() const {
				bool successful = true;
				for (size_t i = 0; i < this->status_codes.size(); ++i) {
					if (this->status_codes[i] != 0) {
						successful = false;
					}
				}

				return successful;
			}

			std::vector<int> status_codes;
		};

		typedef boost::function<int()> GenericCallback;

		ConcurrentTask() {
			this->finished_counter = 0;
		}

		~ConcurrentTask() {
			this->function_callbacks.clear();

			for (size_t i = 0; i < this->future_watchers.size(); ++i) {
				if (this->future_watchers[i] != NULL) {
					delete this->future_watchers[i];
					this->future_watchers[i] = NULL;
				}
			}
			this->future_watchers.clear();
		}

		QString queue_name;

		std::vector<GenericCallback> function_callbacks;
		std::vector<QFutureWatcher<int>* > future_watchers;

		int finished_counter;

		Controller::ConcurrentTask::Status status;

		QString result_slot_name;
	};

	struct ConcurrentTaskDataContainer {
		void reset() {
			this->values.clear();
		};

		QVariant getValue(QString _name) const {
			std::map<QString, QVariant>::const_iterator it = this->values.find(_name);
			if (it != this->values.end()) {
				return it->second;
			}
			else {
				return QVariant();
			}
		}

		void setValue(QString _name, QVariant _value) {
			this->values[_name] = _value;
		}

		std::map<QString, QVariant> values;
	};

	Controller(QObject* _parent = NULL);
	virtual ~Controller();

	QString getPrefix() const;
	void setPrefix(QString _prefix);

	QString getMachineConfigurationBaseDirectoryPath() const;

	QString getMachineConfigurationDirectoryPath() const;

	MachineManager* getMachineManager();
	const MachineManager* getMachineManager() const;

	QString getCurrentMachineName() const;
	void setCurrentMachineName(QString _current_machine_name);
	QStringList retrieveAvailableMachineNames() const;

	void triggerConcurrentComandProcessing(QString _queue_name);

public slots:
	void enqueueConcurrentTask(QString _queue_name, Controller::ConcurrentTask::GenericCallback _function_callback, QString _result_slot_name);
	void enqueueConcurrentTask(QString _queue_name, std::vector<Controller::ConcurrentTask::GenericCallback> _function_callbacka, QString _result_slot_name);
	void processConcurrentCommandFinished();

	void initializeApplication();
	int initializeApplicationConcurrent(QString _queue_name);
	void processInitializeApplicationFinished(Controller::ConcurrentTask::Status _status);

	void uninitializeApplication();
	int uninitializeApplicationConcurrent(QString _queue_name);
	void processUninitializeApplicationFinished(Controller::ConcurrentTask::Status _status);

	void enableAutomaticRead(size_t _line_id, size_t _robot_id);
	int enableAutomaticReadConcurrent(QString _queue_name, size_t _line_id, size_t _robot_id);
	void processEnableAutomaticReadFinished(Controller::ConcurrentTask::Status _status);

	void disableAutomaticRead(size_t _line_id, size_t _robot_id);
	int disableAutomaticReadConcurrent(QString _queue_name, size_t _line_id, size_t _robot_id);
	void processDisableAutomaticReadFinished(Controller::ConcurrentTask::Status _status);

	void restart(size_t _line_id, size_t _robot_id);
	int restartConcurrent(QString _queue_name, size_t _line_id, size_t _robot_id);
	void processRestartFinished(Controller::ConcurrentTask::Status _status);

	void processSelectMachineDialogClosed(QString _queue_name, QString _answer, QString _machine_name);

signals:
	void initializeApplicationFinished(bool _status);
	void uninitializeApplicationFinished(bool _status);
	void enableAutomaticReadFinished(bool _status);
	void disableAutomaticReadFinished(bool _status);
	void restartFinished(bool _status);

	void applicationErrorOccured(ApplicationError _application_error);

	void requestSelectMachineDialog(QString _queue_name, QStringList _machine_names);

	void machineRepresentationChangedd(MachineRepresentation _machine_representation);
	void robotRepresentationChangedd(RobotRepresentation _robot_representation);

private:
	void addConcurrentTaskQueue(QString _queue_name);

	mutable QReadWriteLock data_lock;
		QString prefix;
		QString current_machine_name;

	//VirtualPLC* plc;

	QThread* machine_manager_thread;

	MachineManager* machine_manager;

	mutable QReadWriteLock concurrent_tasks_lock;
	std::map<QString, int> concurrent_task_processing_triggerd_flag;
	std::map<QString, std::queue<ConcurrentTask*> > concurrent_task_queues;

	mutable std::map<QString, QMutex*> concurrent_task_wait_condition_mutex;
	std::map<QString, QWaitCondition*> concurrent_task_wait_condition;

	mutable std::map<QString, QReadWriteLock*> concurrent_task_wait_condition_data_lock;
	std::map<QString, ConcurrentTaskDataContainer> concurrent_task_wait_condition_data;
};

}

#endif /* VM_CONTROLLER_H */