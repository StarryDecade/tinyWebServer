#include "sql_connection_pool.h"
#include <cstdlib>
#include <mysql/mysql.h>

connection_pool::connection_pool() {
	this->freeConn = 0;
	this->currConn = 0;
}

connection_pool::~connection_pool() {
	destoryPool();
}

connection_pool *connection_pool::getInstance() {
	static connection_pool connPoll;
	return &connPoll;
}

void connection_pool::init(string url, int port, string user, string passwd, string dbName, int maxConn) {
	this->url = url;
	this->port = port;
	this->user = user;
	this->passwd = passwd;
	this->dbName = dbName;

	lock.lock();

	for (int i = 0; i < maxConn; i++) {
		MYSQL *conn = NULL;
		conn = mysql_init(conn);
		if (conn == NULL) {
			cout << "Error:" << mysql_error(conn) << endl;
			exit(1);
		}

		conn = mysql_real_connect(conn, url.c_str(), user.c_str(), passwd.c_str(), dbName.c_str(), port, NULL, 0);
		if (conn == NULL) {
			cout << "Error:" << mysql_error(conn) << endl;
			exit(1);
		}
		
		connList.push_back(conn);
		freeConn++;
	}

	reserve = sem(freeConn);
	this->maxConn = maxConn;

	lock.unlock();
}

MYSQL *connection_pool::getConn() {
	MYSQL *conn = NULL;

	if (connList.size() == 0)	return NULL;
	reserve.wait();

	lock.lock();

	conn = connList.front();
	connList.pop_front();

	freeConn--;
	currConn++;

	lock.unlock();

	return conn;
}

bool connection_pool::releaseConn(MYSQL *conn) {
	if (conn == NULL) return false;

	lock.lock();

	connList.push_back(conn);
	freeConn++;
	currConn--;

	lock.unlock();

	reserve.post();
	return true;
}

int connection_pool::getFreeConnCount() {
	return this->freeConn;
}

void  connection_pool::destoryPool() {
	lock.lock();

	if (connList.size() > 0) {
		for (auto it = connList.begin(); it != connList.end(); it++) {
			MYSQL *conn = *it;
			mysql_close(conn);
		}

		this->freeConn = 0;
		this->maxConn = 0;
		connList.clear();
	}

	lock.unlock();
}

connectionRAII::connectionRAII(MYSQL **conn, connection_pool *connPool) {
	*conn = connPool->getConn();

	conRAII = *conn;
	poolRAII = connPool;
}

connectionRAII::~connectionRAII() {
	poolRAII->releaseConn(conRAII);
}
