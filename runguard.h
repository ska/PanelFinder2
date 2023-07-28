#ifndef RUNGUARD_H
#define RUNGUARD_H
#include <QSharedMemory>
#include <QSystemSemaphore>
#include <QCryptographicHash>

#include "common.h"

class RunGuard
{

public:
    RunGuard( const QString& key );
    ~RunGuard();

    bool isAnotherRunning();
    bool tryToRun();
    void release();

private:
    const QString key;
    const QString memLockKey;
    const QString sharedmemKey;

    QSharedMemory sharedMem;
    QSystemSemaphore memLock;

    Q_DISABLE_COPY( RunGuard )
};


#endif // RUNGUARD_H
