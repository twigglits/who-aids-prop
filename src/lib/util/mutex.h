#ifndef MUTEX_H

#define MUTEX_H

#include <omp.h>

class Mutex
{
public:
	Mutex();
	~Mutex();

	void lock()									{ omp_set_lock(&m_lock); }
	void unlock()									{ omp_unset_lock(&m_lock); }
private:
	omp_lock_t m_lock;
};

#endif // MUTEX_H
